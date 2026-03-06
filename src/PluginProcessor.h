#pragma once

#include <JuceHeader.h>

class KickBassAssistantAudioProcessor final : public juce::AudioProcessor
{
public:
    struct AnalysisSnapshot
    {
        float conflict = 0.0f;
        float monoCompatibility = 1.0f;
        float lowEndCorrelation = 0.5f;
        float detector = 0.0f;
        float recommendedOffsetMs = 0.0f;
        bool suggestInvertPolarity = false;
        bool sidechainConnected = false;
    };

    KickBassAssistantAudioProcessor();
    ~KickBassAssistantAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    [[nodiscard]] juce::AudioProcessorEditor* createEditor() override;
    [[nodiscard]] bool hasEditor() const override { return true; }

    [[nodiscard]] const juce::String getName() const override;
    [[nodiscard]] bool acceptsMidi() const override { return false; }
    [[nodiscard]] bool producesMidi() const override { return false; }
    [[nodiscard]] bool isMidiEffect() const override { return false; }
    [[nodiscard]] double getTailLengthSeconds() const override { return 0.0; }

    [[nodiscard]] int getNumPrograms() override { return 1; }
    [[nodiscard]] int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    [[nodiscard]] const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    [[nodiscard]] juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters_; }
    [[nodiscard]] AnalysisSnapshot getAnalysisSnapshot() const;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    static constexpr float maxOffsetMs = 5.0f;

    void updateCrossover (float newCrossoverHz);
    void ensureScratchSpace (int mainChannels, int sidechainChannels, int numSamples);
    void applyBassDelay (const juce::AudioBuffer<float>& input, juce::AudioBuffer<float>& output, int delaySamples);
    void updateAnalysis (int numSamples, int mainChannels);
    static float msToCoefficient (float milliseconds, double sampleRate);
    static float msToSamples (float milliseconds, double sampleRate);
    static float clamp01 (float value);

    juce::AudioProcessorValueTreeState parameters_;

    std::atomic<float>* listenParam_ = nullptr;
    std::atomic<float>* polarityParam_ = nullptr;
    std::atomic<float>* offsetParam_ = nullptr;
    std::atomic<float>* crossoverParam_ = nullptr;
    std::atomic<float>* duckAmountParam_ = nullptr;
    std::atomic<float>* attackParam_ = nullptr;
    std::atomic<float>* releaseParam_ = nullptr;
    std::atomic<float>* outputTrimParam_ = nullptr;

    juce::dsp::LinkwitzRileyFilter<float> bassLowpass_;
    juce::dsp::LinkwitzRileyFilter<float> bassHighpass_;
    juce::dsp::LinkwitzRileyFilter<float> kickLowpass_;

    juce::AudioBuffer<float> delayBuffer_;
    juce::AudioBuffer<float> delayedBassBuffer_;
    juce::AudioBuffer<float> kickBuffer_;
    juce::AudioBuffer<float> analysisOutputLowBuffer_;
    juce::HeapBlock<float> analysisBassLow_;
    juce::HeapBlock<float> analysisKickLow_;
    juce::HeapBlock<float> analysisOutputLowMono_;

    juce::LinearSmoothedValue<float> outputTrimGain_;

    double currentSampleRate_ = 44100.0;
    int maxOffsetSamples_ = 0;
    int delayWritePosition_ = 0;
    float currentCrossoverHz_ = -1.0f;
    float detectorEnvelope_ = 0.0f;

    std::atomic<float> conflictMeter_ { 0.0f };
    std::atomic<float> monoMeter_ { 1.0f };
    std::atomic<float> correlationMeter_ { 0.5f };
    std::atomic<float> detectorMeter_ { 0.0f };
    std::atomic<float> recommendedOffsetMs_ { 0.0f };
    std::atomic<int> suggestInvertPolarity_ { 0 };
    std::atomic<int> sidechainConnected_ { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KickBassAssistantAudioProcessor)
};
