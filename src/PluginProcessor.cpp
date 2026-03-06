#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr auto listenBass = 0;
constexpr auto listenKick = 1;
constexpr auto listenBoth = 2;
constexpr auto listenMono = 3;
constexpr auto listenDelta = 4;
constexpr float analysisSmoothing = 0.2f;
constexpr float epsilon = 1.0e-9f;

bool isMonoOrStereoSet (const juce::AudioChannelSet& channelSet)
{
    return channelSet == juce::AudioChannelSet::mono() || channelSet == juce::AudioChannelSet::stereo();
}

float smoothMeter (std::atomic<float>& target, float nextValue)
{
    const auto previous = target.load();
    const auto smoothed = previous + (nextValue - previous) * analysisSmoothing;
    target.store (smoothed);
    return smoothed;
}
} // namespace

KickBassAssistantAudioProcessor::KickBassAssistantAudioProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withInput ("Sidechain", juce::AudioChannelSet::stereo(), false)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_ (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    listenParam_ = parameters_.getRawParameterValue ("listen");
    polarityParam_ = parameters_.getRawParameterValue ("polarity");
    offsetParam_ = parameters_.getRawParameterValue ("offsetMs");
    crossoverParam_ = parameters_.getRawParameterValue ("crossoverHz");
    duckAmountParam_ = parameters_.getRawParameterValue ("duckAmount");
    attackParam_ = parameters_.getRawParameterValue ("attackMs");
    releaseParam_ = parameters_.getRawParameterValue ("releaseMs");
    outputTrimParam_ = parameters_.getRawParameterValue ("outputTrimDb");
}

const juce::String KickBassAssistantAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

juce::AudioProcessorValueTreeState::ParameterLayout KickBassAssistantAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    parameters.push_back (std::make_unique<juce::AudioParameterChoice> (
        "language",
        "Language",
        juce::StringArray { "English", "Chinese" },
        0));

    parameters.push_back (std::make_unique<juce::AudioParameterChoice> (
        "listen",
        "Listen",
        juce::StringArray { "Bass", "Kick", "Both", "Mono", "Delta" },
        listenBass));

    parameters.push_back (std::make_unique<juce::AudioParameterBool> (
        "polarity",
        "Polarity",
        false));

    parameters.push_back (std::make_unique<juce::AudioParameterFloat> (
        "offsetMs",
        "Offset",
        juce::NormalisableRange<float> (-maxOffsetMs, maxOffsetMs, 0.01f),
        0.0f,
        "ms"));

    parameters.push_back (std::make_unique<juce::AudioParameterFloat> (
        "crossoverHz",
        "Crossover",
        juce::NormalisableRange<float> (40.0f, 180.0f, 0.1f, 0.35f),
        90.0f,
        "Hz"));

    parameters.push_back (std::make_unique<juce::AudioParameterFloat> (
        "duckAmount",
        "Duck Amount",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.001f),
        0.35f));

    parameters.push_back (std::make_unique<juce::AudioParameterFloat> (
        "attackMs",
        "Attack",
        juce::NormalisableRange<float> (1.0f, 60.0f, 0.1f, 0.45f),
        8.0f,
        "ms"));

    parameters.push_back (std::make_unique<juce::AudioParameterFloat> (
        "releaseMs",
        "Release",
        juce::NormalisableRange<float> (20.0f, 400.0f, 0.1f, 0.45f),
        120.0f,
        "ms"));

    parameters.push_back (std::make_unique<juce::AudioParameterFloat> (
        "outputTrimDb",
        "Output Trim",
        juce::NormalisableRange<float> (-18.0f, 6.0f, 0.01f),
        0.0f,
        "dB"));

    return { parameters.begin(), parameters.end() };
}

void KickBassAssistantAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate_ = sampleRate;
    maxOffsetSamples_ = static_cast<int> (std::ceil (msToSamples (maxOffsetMs, sampleRate)));
    delayWritePosition_ = 0;
    detectorEnvelope_ = 0.0f;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (juce::jmax (2, getTotalNumInputChannels()));

    bassLowpass_.reset();
    bassHighpass_.reset();
    kickLowpass_.reset();

    bassLowpass_.prepare (spec);
    bassHighpass_.prepare (spec);
    kickLowpass_.prepare (spec);

    bassLowpass_.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    bassHighpass_.setType (juce::dsp::LinkwitzRileyFilterType::highpass);
    kickLowpass_.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);

    currentCrossoverHz_ = -1.0f;
    updateCrossover (crossoverParam_->load());

    const auto delayBufferSize = (maxOffsetSamples_ * 2) + samplesPerBlock + 8;
    delayBuffer_.setSize (juce::jmax (2, getTotalNumInputChannels()), delayBufferSize);
    delayBuffer_.clear();

    delayedBassBuffer_.setSize (juce::jmax (2, getMainBusNumInputChannels()), samplesPerBlock);
    kickBuffer_.setSize (2, samplesPerBlock);
    analysisOutputLowBuffer_.setSize (juce::jmax (2, getMainBusNumInputChannels()), samplesPerBlock);

    analysisBassLow_.allocate (static_cast<size_t> (samplesPerBlock), true);
    analysisKickLow_.allocate (static_cast<size_t> (samplesPerBlock), true);
    analysisOutputLowMono_.allocate (static_cast<size_t> (samplesPerBlock), true);

    outputTrimGain_.reset (sampleRate, 0.03);
    outputTrimGain_.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (outputTrimParam_->load()));

    setLatencySamples (maxOffsetSamples_);
}

void KickBassAssistantAudioProcessor::releaseResources()
{
}

bool KickBassAssistantAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainInput = layouts.getChannelSet (true, 0);
    const auto& mainOutput = layouts.getChannelSet (false, 0);

    if (mainInput != mainOutput || ! isMonoOrStereoSet (mainInput))
        return false;

    if (layouts.inputBuses.size() > 1)
    {
        const auto& sidechain = layouts.getChannelSet (true, 1);

        if (! sidechain.isDisabled() && ! isMonoOrStereoSet (sidechain))
            return false;
    }

    return true;
}

void KickBassAssistantAudioProcessor::ensureScratchSpace (int mainChannels, int sidechainChannels, int numSamples)
{
    if (delayedBassBuffer_.getNumChannels() != mainChannels || delayedBassBuffer_.getNumSamples() < numSamples)
        delayedBassBuffer_.setSize (mainChannels, numSamples, false, false, true);

    if (analysisOutputLowBuffer_.getNumChannels() != mainChannels || analysisOutputLowBuffer_.getNumSamples() < numSamples)
        analysisOutputLowBuffer_.setSize (mainChannels, numSamples, false, false, true);

    if (kickBuffer_.getNumChannels() != sidechainChannels || kickBuffer_.getNumSamples() < numSamples)
        kickBuffer_.setSize (juce::jmax (1, sidechainChannels), numSamples, false, false, true);
}

void KickBassAssistantAudioProcessor::updateCrossover (float newCrossoverHz)
{
    if (juce::approximatelyEqual (currentCrossoverHz_, newCrossoverHz))
        return;

    currentCrossoverHz_ = newCrossoverHz;
    bassLowpass_.setCutoffFrequency (newCrossoverHz);
    bassHighpass_.setCutoffFrequency (newCrossoverHz);
    kickLowpass_.setCutoffFrequency (newCrossoverHz);
}

void KickBassAssistantAudioProcessor::applyBassDelay (const juce::AudioBuffer<float>& input,
                                                      juce::AudioBuffer<float>& output,
                                                      int delaySamples)
{
    const auto channels = input.getNumChannels();
    const auto numSamples = input.getNumSamples();
    const auto bufferLength = delayBuffer_.getNumSamples();

    jassert (delaySamples >= 0);
    jassert (bufferLength > delaySamples);

    for (int channel = 0; channel < channels; ++channel)
    {
        const auto* inputData = input.getReadPointer (channel);
        auto* outputData = output.getWritePointer (channel);
        auto* delayData = delayBuffer_.getWritePointer (channel);

        auto writePosition = delayWritePosition_;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            delayData[writePosition] = inputData[sample];

            auto readPosition = writePosition - delaySamples;
            if (readPosition < 0)
                readPosition += bufferLength;

            outputData[sample] = delayData[readPosition];

            ++writePosition;
            if (writePosition >= bufferLength)
                writePosition = 0;
        }
    }

    delayWritePosition_ += numSamples;
    delayWritePosition_ %= delayBuffer_.getNumSamples();
}

void KickBassAssistantAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalInputChannels = getTotalNumInputChannels();
    const auto totalOutputChannels = getTotalNumOutputChannels();

    for (int channel = totalInputChannels; channel < totalOutputChannels; ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    auto mainBuffer = getBusBuffer (buffer, true, 0);
    auto sidechainBuffer = getBusBuffer (buffer, true, 1);

    const auto mainChannels = mainBuffer.getNumChannels();
    const auto sidechainChannels = sidechainBuffer.getNumChannels();
    const auto numSamples = mainBuffer.getNumSamples();

    ensureScratchSpace (mainChannels, sidechainChannels, numSamples);
    updateCrossover (crossoverParam_->load());

    const auto requestedOffsetSamples = static_cast<int> (std::round (msToSamples (offsetParam_->load(), currentSampleRate_)));
    const auto bassDelaySamples = juce::jlimit (0, maxOffsetSamples_ * 2, maxOffsetSamples_ + requestedOffsetSamples);
    const auto attackCoeff = msToCoefficient (attackParam_->load(), currentSampleRate_);
    const auto releaseCoeff = msToCoefficient (releaseParam_->load(), currentSampleRate_);
    const auto duckAmount = clamp01 (duckAmountParam_->load());
    const auto polarity = polarityParam_->load() >= 0.5f;
    const auto listenMode = static_cast<int> (listenParam_->load());

    sidechainConnected_.store (sidechainChannels > 0 ? 1 : 0);

    applyBassDelay (mainBuffer, delayedBassBuffer_, bassDelaySamples);

    if (sidechainChannels > 0)
        kickBuffer_.makeCopyOf (sidechainBuffer, true);
    else
        kickBuffer_.clear();

    outputTrimGain_.setTargetValue (juce::Decibels::decibelsToGain (outputTrimParam_->load()));
    analysisOutputLowBuffer_.clear();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float kickMonoLow = 0.0f;

        if (sidechainChannels > 0)
        {
            for (int channel = 0; channel < sidechainChannels; ++channel)
            {
                const auto kickSample = kickBuffer_.getSample (channel, sample);
                kickMonoLow += kickLowpass_.processSample (channel, kickSample);
            }

            kickMonoLow /= static_cast<float> (sidechainChannels);
        }

        const auto detectorInput = clamp01 (std::abs (kickMonoLow) * 2.0f);

        if (detectorInput > detectorEnvelope_)
            detectorEnvelope_ = detectorInput + attackCoeff * (detectorEnvelope_ - detectorInput);
        else
            detectorEnvelope_ = detectorInput + releaseCoeff * (detectorEnvelope_ - detectorInput);

        const auto duckGain = juce::jlimit (0.0f, 1.0f, 1.0f - (detectorEnvelope_ * duckAmount));
        const auto outputTrim = outputTrimGain_.getNextValue();

        float processedMono = 0.0f;
        float bassLowMono = 0.0f;
        float outputLowMono = 0.0f;
        for (int channel = 0; channel < mainChannels; ++channel)
        {
            const auto delayedSample = delayedBassBuffer_.getSample (channel, sample);
            auto lowBand = bassLowpass_.processSample (channel, delayedSample);
            const auto highBand = bassHighpass_.processSample (channel, delayedSample);

            if (polarity)
                lowBand = -lowBand;

            const auto processedLow = lowBand * duckGain;
            const auto processedBass = (processedLow + highBand) * outputTrim;
            const auto kickOut = sidechainChannels > 0
                                     ? kickBuffer_.getSample (juce::jmin (channel, sidechainChannels - 1), sample)
                                     : 0.0f;

            processedMono += processedBass;
            bassLowMono += lowBand;
            outputLowMono += processedLow;
            analysisOutputLowBuffer_.setSample (channel, sample, processedLow);

            switch (listenMode)
            {
                case listenKick:
                    mainBuffer.setSample (channel, sample, kickOut);
                    break;
                case listenBoth:
                    mainBuffer.setSample (channel, sample, processedBass + (kickOut * 0.75f));
                    break;
                case listenMono:
                    break;
                case listenDelta:
                    mainBuffer.setSample (channel, sample, delayedSample - processedBass);
                    break;
                case listenBass:
                default:
                    mainBuffer.setSample (channel, sample, processedBass);
                    break;
            }
        }

        processedMono /= static_cast<float> (juce::jmax (1, mainChannels));
        bassLowMono /= static_cast<float> (juce::jmax (1, mainChannels));
        outputLowMono /= static_cast<float> (juce::jmax (1, mainChannels));

        if (listenMode == listenMono)
        {
            for (int channel = 0; channel < mainChannels; ++channel)
                mainBuffer.setSample (channel, sample, processedMono);
        }

        analysisBassLow_[sample] = bassLowMono;
        analysisKickLow_[sample] = kickMonoLow;
        analysisOutputLowMono_[sample] = outputLowMono;
    }

    updateAnalysis (numSamples, mainChannels);
}

void KickBassAssistantAudioProcessor::updateAnalysis (int numSamples, int mainChannels)
{
    float bassEnergy = 0.0f;
    float kickEnergy = 0.0f;
    float overlap = 0.0f;
    float dot = 0.0f;
    float monoCompatibility = 1.0f;

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto bassLow = analysisOutputLowMono_[sample];
        const auto kickLow = analysisKickLow_[sample];

        bassEnergy += bassLow * bassLow;
        kickEnergy += kickLow * kickLow;
        overlap += std::abs (bassLow) * std::abs (kickLow);
        dot += bassLow * kickLow;
    }

    float signedCorrelation = 0.0f;
    if (bassEnergy > epsilon && kickEnergy > epsilon)
        signedCorrelation = dot / std::sqrt (bassEnergy * kickEnergy);

    const auto overlapNormalised = overlap / (std::sqrt (bassEnergy * kickEnergy) + epsilon);
    const auto phaseConflict = 0.5f * (1.0f - juce::jlimit (-1.0f, 1.0f, signedCorrelation));
    const auto conflict = clamp01 ((overlapNormalised * 0.65f) + (phaseConflict * 0.35f));

    if (mainChannels > 1)
    {
        float leftEnergy = 0.0f;
        float rightEnergy = 0.0f;
        float stereoDot = 0.0f;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const auto left = analysisOutputLowBuffer_.getSample (0, sample);
            const auto right = analysisOutputLowBuffer_.getSample (1, sample);

            leftEnergy += left * left;
            rightEnergy += right * right;
            stereoDot += left * right;
        }

        if (leftEnergy > epsilon && rightEnergy > epsilon)
        {
            const auto stereoCorrelation = stereoDot / std::sqrt (leftEnergy * rightEnergy);
            monoCompatibility = 0.5f * (juce::jlimit (-1.0f, 1.0f, stereoCorrelation) + 1.0f);
        }
    }

    const auto maxLagSamples = juce::jmin (static_cast<int> (std::ceil (msToSamples (maxOffsetMs, currentSampleRate_))),
                                           juce::jmax (0, numSamples / 2 - 1));

    int bestLag = 0;
    float bestEnvelopeCorrelation = -1.0f;

    for (int lag = -maxLagSamples; lag <= maxLagSamples; ++lag)
    {
        float laggedDot = 0.0f;
        float bassAbsEnergy = 0.0f;
        float kickAbsEnergy = 0.0f;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const auto bassIndex = sample + lag;
            if (bassIndex < 0 || bassIndex >= numSamples)
                continue;

            const auto bassAbs = std::abs (analysisBassLow_[bassIndex]);
            const auto kickAbs = std::abs (analysisKickLow_[sample]);

            laggedDot += bassAbs * kickAbs;
            bassAbsEnergy += bassAbs * bassAbs;
            kickAbsEnergy += kickAbs * kickAbs;
        }

        if (bassAbsEnergy <= epsilon || kickAbsEnergy <= epsilon)
            continue;

        const auto lagCorrelation = laggedDot / std::sqrt (bassAbsEnergy * kickAbsEnergy);
        if (lagCorrelation > bestEnvelopeCorrelation)
        {
            bestEnvelopeCorrelation = lagCorrelation;
            bestLag = lag;
        }
    }

    smoothMeter (conflictMeter_, conflict);
    smoothMeter (monoMeter_, monoCompatibility);
    smoothMeter (correlationMeter_, 0.5f * (juce::jlimit (-1.0f, 1.0f, signedCorrelation) + 1.0f));
    smoothMeter (detectorMeter_, detectorEnvelope_);
    recommendedOffsetMs_.store (static_cast<float> (-bestLag) * 1000.0f / static_cast<float> (currentSampleRate_));
    suggestInvertPolarity_.store (signedCorrelation < -0.15f ? 1 : 0);
}

float KickBassAssistantAudioProcessor::msToCoefficient (float milliseconds, double sampleRate)
{
    const auto safeMilliseconds = juce::jmax (0.001f, milliseconds);
    return std::exp (-1.0f / (0.001f * safeMilliseconds * static_cast<float> (sampleRate)));
}

float KickBassAssistantAudioProcessor::msToSamples (float milliseconds, double sampleRate)
{
    return milliseconds * 0.001f * static_cast<float> (sampleRate);
}

float KickBassAssistantAudioProcessor::clamp01 (float value)
{
    return juce::jlimit (0.0f, 1.0f, value);
}

KickBassAssistantAudioProcessor::AnalysisSnapshot KickBassAssistantAudioProcessor::getAnalysisSnapshot() const
{
    AnalysisSnapshot snapshot;
    snapshot.conflict = conflictMeter_.load();
    snapshot.monoCompatibility = monoMeter_.load();
    snapshot.lowEndCorrelation = correlationMeter_.load();
    snapshot.detector = detectorMeter_.load();
    snapshot.recommendedOffsetMs = recommendedOffsetMs_.load();
    snapshot.suggestInvertPolarity = suggestInvertPolarity_.load() != 0;
    snapshot.sidechainConnected = sidechainConnected_.load() != 0;
    return snapshot;
}

void KickBassAssistantAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters_.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void KickBassAssistantAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary (data, sizeInBytes))
        if (xmlState->hasTagName (parameters_.state.getType()))
            parameters_.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorEditor* KickBassAssistantAudioProcessor::createEditor()
{
    return new KickBassAssistantAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KickBassAssistantAudioProcessor();
}
