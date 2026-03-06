#include "PluginEditor.h"

#include <array>

namespace
{
const auto shellTop = juce::Colour::fromRGB (44, 45, 49);
const auto shellBottom = juce::Colour::fromRGB (23, 23, 25);
const auto cardFill = juce::Colour::fromRGB (48, 49, 54);
const auto cardInner = juce::Colour::fromRGB (31, 32, 36);
const auto cardOutline = juce::Colour::fromRGB (84, 84, 88);
const auto cardHighlight = juce::Colour::fromRGBA (255, 255, 255, 18);
const auto textPrimary = juce::Colour::fromRGB (242, 242, 247);
const auto textSecondary = juce::Colour::fromRGB (174, 174, 178);
const auto textMuted = juce::Colour::fromRGB (99, 99, 102);
const auto accentBlue = juce::Colour::fromRGB (10, 132, 255);
const auto accentGreen = juce::Colour::fromRGB (50, 215, 75);
const auto accentOrange = juce::Colour::fromRGB (255, 159, 10);

enum class UiLanguage
{
    english = 0,
    chinese = 1
};

UiLanguage toUiLanguage (int index)
{
    return index == 1 ? UiLanguage::chinese : UiLanguage::english;
}

juce::String tr (UiLanguage language, const char* english, const char* chinese)
{
    return language == UiLanguage::chinese ? juce::String (juce::CharPointer_UTF8 (chinese)) : juce::String (english);
}

juce::String utf8 (const char* text)
{
    return juce::String (juce::CharPointer_UTF8 (text));
}

juce::Font makeTitleDisplayFont()
{
    return juce::Font (juce::FontOptions (32.0f)
                           .withName ("Didot")
                           .withStyle ("Bold")
                           .withFallbacks (std::vector<juce::String> { "Avenir Next", "Helvetica Neue", "PingFang SC", "Songti SC" })
                           .withKerningFactor (0.035f));
}

juce::Font makeSubtitleDisplayFont (UiLanguage language)
{
    return juce::Font (juce::FontOptions (13.5f)
                           .withName (language == UiLanguage::chinese ? "Songti SC" : "Avenir Next")
                           .withStyle (language == UiLanguage::chinese ? "Regular" : "Demi Bold Italic")
                           .withFallbacks (std::vector<juce::String> { "Avenir Next", "Helvetica Neue", "PingFang SC", "Songti SC" })
                           .withKerningFactor (0.04f)
                           .withHorizontalScale (1.02f));
}

void styleHeaderLabel (juce::Label& label)
{
    label.setFont (juce::FontOptions (12.5f).withStyle ("Bold"));
    label.setColour (juce::Label::textColourId, textSecondary);
    label.setJustificationType (juce::Justification::centredRight);
}

void styleCaption (juce::Label& label)
{
    label.setFont (juce::FontOptions (11.5f).withStyle ("Bold"));
    label.setColour (juce::Label::textColourId, textSecondary);
    label.setJustificationType (juce::Justification::centredLeft);
}

void repopulateComboBox (juce::ComboBox& comboBox, const juce::StringArray& items)
{
    const auto selectedId = juce::jmax (1, comboBox.getSelectedId());
    comboBox.clear (juce::dontSendNotification);
    comboBox.addItemList (items, 1);
    comboBox.setSelectedId (juce::jmin (selectedId, items.size()), juce::dontSendNotification);
}
} // namespace

KickBassAssistantAudioProcessorEditor::ModernLookAndFeel::ModernLookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, cardInner);
    setColour (juce::ComboBox::textColourId, textPrimary);
    setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::arrowColourId, textPrimary);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour::fromRGB (38, 39, 43));
    setColour (juce::PopupMenu::textColourId, textPrimary);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, accentBlue.withAlpha (0.22f));
    setColour (juce::PopupMenu::highlightedTextColourId, textPrimary);
    setColour (juce::Label::textColourId, textPrimary);
}

void KickBassAssistantAudioProcessorEditor::ModernLookAndFeel::drawRotarySlider (juce::Graphics& g,
                                                                                  int x,
                                                                                  int y,
                                                                                  int width,
                                                                                  int height,
                                                                                  float sliderPosProportional,
                                                                                  float rotaryStartAngle,
                                                                                  float rotaryEndAngle,
                                                                                  juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y), static_cast<float> (width), static_cast<float> (height)).reduced (10.0f, 8.0f);
    auto valueArea = bounds.removeFromBottom (24.0f);
    const auto knobSize = juce::jmin (96.0f, bounds.getWidth(), bounds.getHeight());
    auto knobArea = juce::Rectangle<float> (knobSize, knobSize).withCentre (juce::Point<float> (bounds.getCentreX(), bounds.getCentreY() - 3.0f));

    const auto centre = knobArea.getCentre();
    const auto ringRadius = (knobSize * 0.5f) - 10.0f;
    const auto endAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, ringRadius, ringRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);

    g.setColour (juce::Colour::fromRGB (72, 72, 74));
    g.strokePath (track, juce::PathStrokeType (8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, ringRadius, ringRadius, 0.0f, rotaryStartAngle, endAngle, true);
    const auto accent = slider.isEnabled() ? accentBlue : textMuted;
    g.setColour (accent);
    g.strokePath (value, juce::PathStrokeType (8.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (juce::Colour::fromRGB (52, 52, 54));
    g.fillEllipse (juce::Rectangle<float> (ringRadius * 1.3f, ringRadius * 1.3f).withCentre (centre));

    g.setColour (juce::Colours::black.withAlpha (0.28f));
    g.drawEllipse (juce::Rectangle<float> (ringRadius * 1.3f, ringRadius * 1.3f).withCentre (centre), 1.0f);

    g.setColour (juce::Colour::fromRGB (24, 24, 27));
    g.fillEllipse (juce::Rectangle<float> (ringRadius * 0.72f, ringRadius * 0.72f).withCentre (centre));

    juce::Path pointer;
    pointer.startNewSubPath (centre);
    pointer.lineTo (juce::Point<float> (centre.x + std::cos (endAngle) * (ringRadius - 2.0f),
                                        centre.y + std::sin (endAngle) * (ringRadius - 2.0f)));
    g.setColour (accent);
    g.strokePath (pointer, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (textPrimary);
    g.setFont (juce::FontOptions (11.5f).withStyle ("Bold"));
    g.drawFittedText (slider.getTextFromValue (slider.getValue()), valueArea.toNearestInt(), juce::Justification::centred, 1);
}

void KickBassAssistantAudioProcessorEditor::ModernLookAndFeel::drawLinearSlider (juce::Graphics& g,
                                                                                  int x,
                                                                                  int y,
                                                                                  int width,
                                                                                  int height,
                                                                                  float sliderPos,
                                                                                  float minSliderPos,
                                                                                  float maxSliderPos,
                                                                                  const juce::Slider::SliderStyle style,
                                                                                  juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        return;
    }

    auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y), static_cast<float> (width), static_cast<float> (height)).reduced (2.0f, 0.0f);
    auto valueArea = bounds.removeFromRight (68.0f);
    auto trackArea = bounds.withHeight (6.0f).withCentre (juce::Point<float> (bounds.getCentreX(), bounds.getCentreY()));

    g.setColour (juce::Colour::fromRGB (78, 78, 82));
    g.fillRoundedRectangle (trackArea, 3.0f);

    const auto clampedPos = juce::jlimit (trackArea.getX(), trackArea.getRight(), sliderPos);
    auto fillArea = trackArea;
    fillArea.setWidth (juce::jmax (0.0f, clampedPos - trackArea.getX()));

    juce::ColourGradient fill (accentBlue.brighter (0.05f), fillArea.getTopLeft(),
                               accentBlue.darker (0.1f), fillArea.getBottomRight(), false);
    g.setGradientFill (fill);
    g.fillRoundedRectangle (fillArea, 3.0f);

    auto thumbBounds = juce::Rectangle<float> (14.0f, 14.0f).withCentre (juce::Point<float> (clampedPos, trackArea.getCentreY()));
    g.setColour (textPrimary);
    g.fillEllipse (thumbBounds);
    g.setColour (juce::Colours::black.withAlpha (0.18f));
    g.drawEllipse (thumbBounds, 1.0f);

    g.setColour (textSecondary);
    g.setFont (juce::FontOptions (11.5f).withStyle ("Bold"));
    g.drawText (slider.getTextFromValue (slider.getValue()), valueArea.toNearestInt(), juce::Justification::centredRight, false);
}

void KickBassAssistantAudioProcessorEditor::ModernLookAndFeel::drawComboBox (juce::Graphics& g,
                                                                              int width,
                                                                              int height,
                                                                              bool,
                                                                              int buttonX,
                                                                              int buttonY,
                                                                              int buttonW,
                                                                              int buttonH,
                                                                              juce::ComboBox&)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, static_cast<float> (width), static_cast<float> (height)).reduced (1.0f);
    juce::ColourGradient fill (juce::Colour::fromRGB (46, 47, 52), bounds.getTopLeft(),
                               juce::Colour::fromRGB (27, 28, 31), bounds.getBottomRight(), false);
    g.setGradientFill (fill);
    g.fillRoundedRectangle (bounds, 12.0f);

    g.setColour (cardOutline.withAlpha (0.7f));
    g.drawRoundedRectangle (bounds, 12.0f, 1.0f);

    g.setColour (textPrimary);
    const auto chevronArea = juce::Rectangle<float> (static_cast<float> (buttonX), static_cast<float> (buttonY), static_cast<float> (buttonW), static_cast<float> (buttonH));
    const auto centreX = chevronArea.getCentreX() + 2.0f;
    const auto centreY = chevronArea.getCentreY();
    juce::Path chevron;
    chevron.startNewSubPath (centreX - 4.0f, centreY - 2.0f);
    chevron.lineTo (centreX, centreY + 3.0f);
    chevron.lineTo (centreX + 4.0f, centreY - 2.0f);
    g.strokePath (chevron, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font KickBassAssistantAudioProcessorEditor::ModernLookAndFeel::getComboBoxFont (juce::ComboBox&) 
{
    return juce::Font (juce::FontOptions (13.0f).withStyle ("Bold"));
}

void KickBassAssistantAudioProcessorEditor::ModernLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (14, 1, box.getWidth() - 42, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
    label.setColour (juce::Label::textColourId, textPrimary);
    label.setJustificationType (juce::Justification::centredLeft);
}

void KickBassAssistantAudioProcessorEditor::ModernLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                                                                      juce::Button& button,
                                                                                      const juce::Colour&,
                                                                                      bool shouldDrawButtonAsHighlighted,
                                                                                      bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
    auto fill = button.getToggleState() ? accentBlue.withAlpha (0.18f) : juce::Colour::fromRGB (32, 33, 37);
    if (shouldDrawButtonAsHighlighted)
        fill = fill.brighter (0.08f);
    if (shouldDrawButtonAsDown)
        fill = fill.darker (0.08f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 12.0f);

    g.setColour (button.getToggleState() ? accentBlue : cardOutline.withAlpha (0.72f));
    g.drawRoundedRectangle (bounds, 12.0f, 1.0f);
}

juce::Font KickBassAssistantAudioProcessorEditor::ModernLookAndFeel::getTextButtonFont (juce::TextButton&, int)
{
    return juce::Font (juce::FontOptions (13.0f).withStyle ("Bold"));
}

void KickBassAssistantAudioProcessorEditor::ModernLookAndFeel::drawButtonText (juce::Graphics& g,
                                                                                juce::TextButton& button,
                                                                                bool,
                                                                                bool)
{
    g.setColour (button.getToggleState() ? accentBlue : textPrimary);
    g.setFont (getTextButtonFont (button, button.getHeight()));
    g.drawFittedText (button.getButtonText(), button.getLocalBounds().reduced (14, 0), juce::Justification::centred, 1);
}

KickBassAssistantAudioProcessorEditor::MeterComponent::MeterComponent (juce::String title, juce::Colour activeColour)
    : title_ (std::move (title)),
      activeColour_ (activeColour)
{
}

void KickBassAssistantAudioProcessorEditor::MeterComponent::setTitle (juce::String newTitle)
{
    if (title_ == newTitle)
        return;

    title_ = std::move (newTitle);
    repaint();
}

void KickBassAssistantAudioProcessorEditor::MeterComponent::setValue (float newValue)
{
    const auto clamped = juce::jlimit (0.0f, 1.0f, newValue);
    if (std::abs (clamped - value_) < 0.001f)
        return;

    value_ = clamped;
    repaint();
}

void KickBassAssistantAudioProcessorEditor::MeterComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    auto headerArea = bounds.removeFromTop (18.0f);
    auto meterArea = bounds.reduced (0.0f, 10.0f);

    g.setColour (textSecondary);
    g.setFont (juce::FontOptions (11.5f).withStyle ("Bold"));
    g.drawText (title_, headerArea.removeFromLeft (headerArea.getWidth() * 0.65f).toNearestInt(), juce::Justification::centredLeft, false);

    g.setColour (textMuted);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText (juce::String (juce::roundToInt (value_ * 100.0f)) + "%", headerArea.toNearestInt(), juce::Justification::centredRight, false);

    g.setColour (juce::Colour::fromRGB (44, 45, 49));
    g.fillRoundedRectangle (meterArea, 9.0f);

    auto fill = meterArea.reduced (1.5f);
    fill.setWidth (fill.getWidth() * value_);

    juce::ColourGradient fillGradient (activeColour_.brighter (0.12f), fill.getTopLeft(), activeColour_.darker (0.1f), fill.getBottomRight(), false);
    g.setGradientFill (fillGradient);
    g.fillRoundedRectangle (fill, 8.0f);

    g.setColour (juce::Colours::white.withAlpha (0.05f));
    g.drawRoundedRectangle (meterArea, 9.0f, 1.0f);
}

void KickBassAssistantAudioProcessorEditor::styleSectionLabel (juce::Label& label, const juce::String& text)
{
    label.setText (text, juce::dontSendNotification);
    label.setFont (juce::FontOptions (13.0f).withStyle ("Bold"));
    label.setColour (juce::Label::textColourId, textSecondary);
    label.setJustificationType (juce::Justification::centredLeft);
}

void KickBassAssistantAudioProcessorEditor::styleRotarySlider (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accentBlue);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setMouseDragSensitivity (200);
}

KickBassAssistantAudioProcessorEditor::KickBassAssistantAudioProcessorEditor (KickBassAssistantAudioProcessor& audioProcessor)
    : AudioProcessorEditor (&audioProcessor),
      processor_ (audioProcessor),
      languageAttachment_ (audioProcessor.getValueTreeState(), "language", languageBox_),
      listenAttachment_ (audioProcessor.getValueTreeState(), "listen", listenBox_),
      polarityAttachment_ (audioProcessor.getValueTreeState(), "polarity", polarityButton_),
      offsetAttachment_ (audioProcessor.getValueTreeState(), "offsetMs", offsetSlider_),
      crossoverAttachment_ (audioProcessor.getValueTreeState(), "crossoverHz", crossoverSlider_),
      duckAttachment_ (audioProcessor.getValueTreeState(), "duckAmount", duckSlider_),
      attackAttachment_ (audioProcessor.getValueTreeState(), "attackMs", attackSlider_),
      releaseAttachment_ (audioProcessor.getValueTreeState(), "releaseMs", releaseSlider_),
      outputTrimAttachment_ (audioProcessor.getValueTreeState(), "outputTrimDb", outputTrimSlider_)
{
    setLookAndFeel (&lookAndFeel_);
    setSize (1040, 720);

    styleSectionLabel (controlSectionLabel_, "Controls");
    styleSectionLabel (analysisSectionLabel_, "Analysis");
    styleSectionLabel (helpSectionLabel_, "Help");

    titleLabel_.setText ("KickBass Assistant", juce::dontSendNotification);
    titleLabel_.setFont (makeTitleDisplayFont());
    titleLabel_.setColour (juce::Label::textColourId, textPrimary);
    titleLabel_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel_);

    styleHeaderLabel (languageLabel_);
    addAndMakeVisible (languageLabel_);

    subtitleLabel_.setText ("Low-end interaction tool for bass + sidechain kick", juce::dontSendNotification);
    subtitleLabel_.setFont (makeSubtitleDisplayFont (UiLanguage::english));
    subtitleLabel_.setColour (juce::Label::textColourId, textSecondary);
    subtitleLabel_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (subtitleLabel_);

    addAndMakeVisible (controlSectionLabel_);
    addAndMakeVisible (analysisSectionLabel_);
    addAndMakeVisible (helpSectionLabel_);

    listenBox_.addItemList (juce::StringArray { "Bass", "Kick", "Both", "Mono", "Delta" }, 1);
    addAndMakeVisible (listenBox_);

    addAndMakeVisible (languageBox_);

    addAndMakeVisible (polarityButton_);
    polarityButton_.setClickingTogglesState (true);

    styleRotarySlider (offsetSlider_);
    styleRotarySlider (crossoverSlider_);
    styleRotarySlider (duckSlider_);
    styleRotarySlider (attackSlider_);
    styleRotarySlider (releaseSlider_);
    styleRotarySlider (outputTrimSlider_);

    addAndMakeVisible (offsetSlider_);
    addAndMakeVisible (crossoverSlider_);
    addAndMakeVisible (duckSlider_);
    addAndMakeVisible (attackSlider_);
    addAndMakeVisible (releaseSlider_);
    addAndMakeVisible (outputTrimSlider_);

    styleCaption (listenLabel_);
    styleCaption (offsetLabel_);
    styleCaption (crossoverLabel_);
    styleCaption (duckLabel_);
    styleCaption (attackLabel_);
    styleCaption (releaseLabel_);
    styleCaption (outputTrimLabel_);

    addAndMakeVisible (listenLabel_);
    addAndMakeVisible (offsetLabel_);
    addAndMakeVisible (crossoverLabel_);
    addAndMakeVisible (duckLabel_);
    addAndMakeVisible (attackLabel_);
    addAndMakeVisible (releaseLabel_);
    addAndMakeVisible (outputTrimLabel_);

    addAndMakeVisible (conflictMeter_);
    addAndMakeVisible (monoMeter_);
    addAndMakeVisible (correlationMeter_);
    addAndMakeVisible (detectorMeter_);

    sidechainStatusLabel_.setColour (juce::Label::textColourId, textPrimary);
    sidechainStatusLabel_.setFont (juce::FontOptions (13.0f).withStyle ("Bold"));
    sidechainStatusLabel_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (sidechainStatusLabel_);

    offsetHintLabel_.setColour (juce::Label::textColourId, textSecondary);
    offsetHintLabel_.setFont (juce::FontOptions (13.0f));
    offsetHintLabel_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (offsetHintLabel_);

    polarityHintLabel_.setColour (juce::Label::textColourId, textSecondary);
    polarityHintLabel_.setFont (juce::FontOptions (13.0f));
    polarityHintLabel_.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (polarityHintLabel_);

    helpTextLabel_.setColour (juce::Label::textColourId, textSecondary);
    helpTextLabel_.setFont (juce::FontOptions (13.5f));
    helpTextLabel_.setJustificationType (juce::Justification::topLeft);
    helpTextLabel_.setMinimumHorizontalScale (1.0f);
    addAndMakeVisible (helpTextLabel_);

    languageParam_ = processor_.getValueTreeState().getRawParameterValue ("language");
    jassert (languageParam_ != nullptr);

    repopulateComboBox (languageBox_, juce::StringArray { "English", utf8 ("中文") });
    repopulateComboBox (listenBox_, juce::StringArray { "Bass", "Kick", "Both", "Mono", "Delta" });
    updateLanguageStrings();

    addMouseListener (this, true);
    startTimerHz (30);
    syncAnalysisLabels();
}

KickBassAssistantAudioProcessorEditor::~KickBassAssistantAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void KickBassAssistantAudioProcessorEditor::registerHelpTarget (juce::Component& component, juce::String text)
{
    helpEntries_.emplace_back (&component, std::move (text));
}

void KickBassAssistantAudioProcessorEditor::configureHelpEntries()
{
    helpEntries_.clear();

    const auto language = toUiLanguage (currentLanguageIndex_);
    registerHelpTarget (languageBox_, tr (language,
                                          "Language switches the full UI between English and Chinese. The selection is saved with the plugin state.",
                                          "Language 用来切换整套界面的中英文显示，切换结果会跟随工程一起保存。"));
    registerHelpTarget (languageLabel_, tr (language,
                                            "Language switches the full UI between English and Chinese. The selection is saved with the plugin state.",
                                            "Language 用来切换整套界面的中英文显示，切换结果会跟随工程一起保存。"));

    registerHelpTarget (listenBox_, tr (language,
                                        "Listen chooses what you monitor. Bass = processed bass, Kick = sidechain only, Both = hear the interaction, Mono = check low end in mono, Delta = hear only what the plugin changed.",
                                        "Listen 用来切换监听方式。Bass 是处理后的 bass，Kick 是只听侧链 kick，Both 是一起听冲突，Mono 用来检查单声道低频，Delta 只听插件改掉的部分。"));
    registerHelpTarget (listenLabel_, tr (language,
                                          "Listen chooses what you monitor. Bass = processed bass, Kick = sidechain only, Both = hear the interaction, Mono = check low end in mono, Delta = hear only what the plugin changed.",
                                          "Listen 用来切换监听方式。Bass 是处理后的 bass，Kick 是只听侧链 kick，Both 是一起听冲突，Mono 用来检查单声道低频，Delta 只听插件改掉的部分。"));

    registerHelpTarget (polarityButton_, tr (language,
                                             "Flip Low Polarity inverts only the low band. Try this first when the kick and bass cancel each other or the low end gets thin in mono.",
                                             "Flip Low Polarity 只会反转低频段极性。当 kick 和 bass 互相抵消，或者单声道下低频变薄时，先试这个开关。"));

    const auto offsetHelp = tr (language,
                                "Offset moves the bass slightly earlier or later against the kick. Positive values make the bass later, negative values make it earlier. Small moves like 0.2 to 1.0 ms are usually enough.",
                                "Offset 用来微调 bass 相对 kick 的前后位置。正值让 bass 更晚，负值让 bass 更早。通常只需要 0.2 到 1.0 ms 这样的小范围调整。");
    registerHelpTarget (offsetSlider_, offsetHelp);
    registerHelpTarget (offsetLabel_, offsetHelp);

    const auto crossoverHelp = tr (language,
                                   "Crossover sets the frequency below which the plugin analyzes, flips polarity, and ducks the bass. A solid starting range is 80 to 100 Hz.",
                                   "Crossover 决定插件从哪个频率以下开始做分析、极性翻转和 ducking。常用起点是 80 到 100 Hz。");
    registerHelpTarget (crossoverSlider_, crossoverHelp);
    registerHelpTarget (crossoverLabel_, crossoverHelp);

    const auto duckHelp = tr (language,
                              "Duck Amount controls how much the kick pushes the bass low end down. More ducking makes the kick clearer, but too much can hollow out the bass.",
                              "Duck Amount 控制 kick 出现时 bass 低频被压下去多少。数值越大，kick 越清楚，但太大可能会让 bass 变空。");
    registerHelpTarget (duckSlider_, duckHelp);
    registerHelpTarget (duckLabel_, duckHelp);

    const auto attackHelp = tr (language,
                                "Attack controls how fast ducking engages. Faster attack makes the kick transient cut through more clearly, but too fast can sound stiff.",
                                "Attack 控制 ducking 启动速度。更快会让 kick 前沿更清楚，但太快有时会显得发硬。");
    registerHelpTarget (attackSlider_, attackHelp);
    registerHelpTarget (attackLabel_, attackHelp);

    const auto releaseHelp = tr (language,
                                 "Release controls how fast the bass low end comes back after the kick. Too short can pump, too long can leave the bass weak. Start around 100 to 160 ms.",
                                 "Release 控制 kick 过去之后 bass 低频恢复的速度。太短会抽动，太长会让 bass 回不来。建议先从 100 到 160 ms 开始。");
    registerHelpTarget (releaseSlider_, releaseHelp);
    registerHelpTarget (releaseLabel_, releaseHelp);

    const auto outputHelp = tr (language,
                                "Output Trim is the final gain stage. Use it to loudness-match against bypass before deciding whether the processing is actually better.",
                                "Output Trim 是最后一级输出补偿。先把处理前后的音量对齐，再判断处理结果是不是更好。");
    registerHelpTarget (outputTrimSlider_, outputHelp);
    registerHelpTarget (outputTrimLabel_, outputHelp);

    registerHelpTarget (conflictMeter_, tr (language,
                                            "Conflict shows how much kick and bass low-end overlap is happening. Lower is not always better. The goal is enough separation without losing weight.",
                                            "Conflict 表示 kick 和 bass 的低频重叠程度。不是越低越好，目标是在分离度和厚度之间找到平衡。"));
    registerHelpTarget (monoMeter_, tr (language,
                                        "Mono shows how stable the low end is when collapsed to mono. If it drops and the low end gets thinner, try Polarity first, then fine-tune Offset.",
                                        "Mono 表示低频折叠成单声道后的稳定程度。如果这里变低而且听感变薄，先试 Polarity，再微调 Offset。"));
    registerHelpTarget (correlationMeter_, tr (language,
                                               "Correlation shows how aligned the kick and bass low bands are. Lower values can indicate more fighting or cancellation. Mid to high values are usually safer.",
                                               "Correlation 表示 kick 和 bass 低频的同向程度。数值偏低通常说明更容易打架或抵消，中高值一般更稳。"));
    registerHelpTarget (detectorMeter_, tr (language,
                                            "Detector shows how strongly the kick is triggering the ducking stage right now. Higher detector values mean more low-band reduction on the bass.",
                                            "Detector 表示当前 kick 对 ducking 的触发强度。数值越高，bass 低频被压下去的程度越大。"));

    registerHelpTarget (sidechainStatusLabel_, tr (language,
                                                   "This tells you whether the plugin is receiving the kick sidechain. Without sidechain input, the analysis and ducking results are not meaningful.",
                                                   "这里会提示插件是否真的收到了 kick 的 sidechain。没有 sidechain 输入时，分析和 ducking 都没有参考意义。"));
    registerHelpTarget (offsetHintLabel_, tr (language,
                                              "Alignment hint is a suggested Offset starting point based on the current analysis window. Use it as a first guess, then trust your ears.",
                                              "Alignment hint 是根据当前分析窗口给出的 Offset 起始建议。先把它当作起点，最后还是以听感为准。"));
    registerHelpTarget (polarityHintLabel_, tr (language,
                                                "Polarity hint suggests whether flipping the low band may help. Keep it only if the low end feels more solid and mono holds up better.",
                                                "Polarity hint 会提示是否值得切换低频极性。只有当低频更扎实、单声道更稳时，才保留这个设置。"));
}

void KickBassAssistantAudioProcessorEditor::updateLanguageStrings()
{
    const auto nextLanguageIndex = languageParam_ != nullptr ? juce::roundToInt (languageParam_->load()) : 0;
    if (nextLanguageIndex == currentLanguageIndex_)
        return;

    currentLanguageIndex_ = nextLanguageIndex;
    const auto language = toUiLanguage (currentLanguageIndex_);
    languageBox_.setSelectedId (currentLanguageIndex_ + 1, juce::dontSendNotification);
    titleLabel_.setFont (makeTitleDisplayFont());
    subtitleLabel_.setFont (makeSubtitleDisplayFont (language));

    languageLabel_.setText (tr (language, "Language", "语言"), juce::dontSendNotification);
    subtitleLabel_.setText (tr (language, "Low-end interaction tool for bass + sidechain kick", "用于 bass 与 sidechain kick 低频协同处理的工具"),
                            juce::dontSendNotification);

    styleSectionLabel (controlSectionLabel_, tr (language, "Controls", "控制"));
    styleSectionLabel (analysisSectionLabel_, tr (language, "Analysis", "分析"));
    styleSectionLabel (helpSectionLabel_, tr (language, "Help", "说明"));

    repopulateComboBox (listenBox_, language == UiLanguage::chinese
                                       ? juce::StringArray { "Bass", "Kick", utf8 ("同时"), utf8 ("单声道"), utf8 ("差值") }
                                       : juce::StringArray { "Bass", "Kick", "Both", "Mono", "Delta" });

    listenLabel_.setText (tr (language, "Listen", "监听"), juce::dontSendNotification);
    offsetLabel_.setText (tr (language, "Offset", "偏移"), juce::dontSendNotification);
    crossoverLabel_.setText (tr (language, "Crossover", "分频点"), juce::dontSendNotification);
    duckLabel_.setText (tr (language, "Duck", "压低"), juce::dontSendNotification);
    attackLabel_.setText (tr (language, "Attack", "起速"), juce::dontSendNotification);
    releaseLabel_.setText (tr (language, "Release", "释放"), juce::dontSendNotification);
    outputTrimLabel_.setText (tr (language, "Output", "输出"), juce::dontSendNotification);
    polarityButton_.setButtonText (tr (language, "Flip Low Polarity", "翻转低频极性"));

    conflictMeter_.setTitle (tr (language, "Conflict", "冲突"));
    monoMeter_.setTitle (tr (language, "Mono", "单声道"));
    correlationMeter_.setTitle (tr (language, "Correlation", "相关性"));
    detectorMeter_.setTitle (tr (language, "Detector", "检测器"));

    defaultHelpText_ = tr (language,
                           "Hover over a control or meter to see what it does and when to adjust it. Start with Listen = Both, Crossover = 90 Hz, Duck = 0.35, Attack = 8 ms, Release = 120 ms, then fine-tune Offset and Polarity by ear.",
                           "把鼠标移到任意参数或分析条上，就能在这里看到对应说明。建议先从 Listen = 同时、Crossover = 90 Hz、Duck = 0.35、Attack = 8 ms、Release = 120 ms 开始，再根据听感微调 Offset 和极性。");

    configureHelpEntries();
    updateHelpTextForComponent (activeHelpComponent_);
}

juce::String KickBassAssistantAudioProcessorEditor::findHelpTextForComponent (const juce::Component* component) const
{
    for (auto current = component; current != nullptr; current = current->getParentComponent())
    {
        for (const auto& entry : helpEntries_)
            if (entry.first == current)
                return entry.second;
    }

    return defaultHelpText_;
}

void KickBassAssistantAudioProcessorEditor::updateHelpTextForComponent (const juce::Component* component)
{
    activeHelpComponent_ = component;
    helpTextLabel_.setText (findHelpTextForComponent (component), juce::dontSendNotification);
}

void KickBassAssistantAudioProcessorEditor::mouseEnter (const juce::MouseEvent& event)
{
    updateHelpTextForComponent (event.originalComponent != nullptr ? event.originalComponent : event.eventComponent);
}

void KickBassAssistantAudioProcessorEditor::mouseMove (const juce::MouseEvent& event)
{
    updateHelpTextForComponent (event.originalComponent != nullptr ? event.originalComponent : event.eventComponent);
}

void KickBassAssistantAudioProcessorEditor::mouseExit (const juce::MouseEvent& event)
{
    if (event.eventComponent == this || event.originalComponent == this)
    {
        activeHelpComponent_ = nullptr;
        helpTextLabel_.setText (defaultHelpText_, juce::dontSendNotification);
    }
}

void KickBassAssistantAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient background (shellTop, 0.0f, 0.0f, shellBottom, 0.0f, static_cast<float> (getHeight()), false);
    background.addColour (0.42, juce::Colour::fromRGB (34, 35, 38));
    g.setGradientFill (background);
    g.fillAll();

    auto whiteGlow = getLocalBounds().toFloat().withSizeKeepingCentre (getWidth() * 0.92f, getHeight() * 0.48f)
                                              .translated (0.0f, -static_cast<float> (getHeight()) * 0.18f);
    juce::ColourGradient topGlow (juce::Colours::white.withAlpha (0.08f), whiteGlow.getCentreX(), whiteGlow.getY(),
                                  juce::Colours::transparentBlack, whiteGlow.getCentreX(), whiteGlow.getBottom(), true);
    g.setGradientFill (topGlow);
    g.fillEllipse (whiteGlow);

    auto blueGlow = getLocalBounds().toFloat().withSizeKeepingCentre (getWidth() * 0.44f, getHeight() * 0.28f)
                                             .translated (0.0f, static_cast<float> (getHeight()) * 0.02f);
    juce::ColourGradient accentGlow (accentBlue.withAlpha (0.12f), blueGlow.getCentreX(), blueGlow.getCentreY(),
                                     juce::Colours::transparentBlack, blueGlow.getRight(), blueGlow.getBottom(), true);
    g.setGradientFill (accentGlow);
    g.fillEllipse (blueGlow);

    auto drawPanel = [&g] (juce::Rectangle<float> bounds, float cornerSize)
    {
        if (bounds.isEmpty())
            return;

        auto shadowBounds = bounds.translated (0.0f, 3.0f);
        g.setColour (juce::Colours::black.withAlpha (0.18f));
        g.fillRoundedRectangle (shadowBounds, cornerSize);

        juce::ColourGradient panelGradient (juce::Colour::fromRGBA (57, 58, 64, 232), bounds.getTopLeft(),
                                            juce::Colour::fromRGBA (28, 29, 32, 244), bounds.getBottomRight(), false);
        panelGradient.addColour (0.4, juce::Colour::fromRGBA (46, 47, 53, 236));
        g.setGradientFill (panelGradient);
        g.fillRoundedRectangle (bounds, cornerSize);

        auto highlight = bounds.reduced (1.5f);
        highlight.setHeight (juce::jmin (highlight.getHeight() * 0.38f, 60.0f));
        juce::ColourGradient sheen (juce::Colours::white.withAlpha (0.065f), highlight.getCentreX(), highlight.getY(),
                                    juce::Colours::transparentBlack, highlight.getCentreX(), highlight.getBottom(), false);
        g.setGradientFill (sheen);
        g.fillRoundedRectangle (highlight, juce::jmax (cornerSize - 2.0f, 1.0f));

        g.setColour (cardOutline.withAlpha (0.52f));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    };

    drawPanel (headerPanelBounds_, 24.0f);
    drawPanel (controlsPanelBounds_, 24.0f);
    drawPanel (analysisPanelBounds_, 24.0f);
    drawPanel (helpPanelBounds_, 24.0f);

    auto drawInsetCard = [&g] (juce::Rectangle<int> area, float cornerSize)
    {
        if (area.isEmpty())
            return;

        auto bounds = area.toFloat();
        juce::ColourGradient fill (juce::Colour::fromRGBA (59, 60, 66, 58), bounds.getTopLeft(),
                                   juce::Colour::fromRGBA (33, 34, 38, 36), bounds.getBottomRight(), false);
        g.setGradientFill (fill);
        g.fillRoundedRectangle (bounds, cornerSize);
        g.setColour (cardOutline.withAlpha (0.26f));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);
    };

    const auto offsetCard = offsetSlider_.getBounds().getUnion (offsetLabel_.getBounds()).expanded (10, 10);
    const auto crossoverCard = crossoverSlider_.getBounds().getUnion (crossoverLabel_.getBounds()).expanded (10, 10);
    const auto duckCard = duckSlider_.getBounds().getUnion (duckLabel_.getBounds()).expanded (10, 10);
    const auto attackCard = attackSlider_.getBounds().getUnion (attackLabel_.getBounds()).expanded (10, 10);
    const auto releaseCard = releaseSlider_.getBounds().getUnion (releaseLabel_.getBounds()).expanded (10, 10);
    const auto outputCard = outputTrimSlider_.getBounds().getUnion (outputTrimLabel_.getBounds()).expanded (10, 10);

    drawInsetCard (offsetCard, 20.0f);
    drawInsetCard (crossoverCard, 20.0f);
    drawInsetCard (duckCard, 20.0f);
    drawInsetCard (attackCard, 20.0f);
    drawInsetCard (releaseCard, 20.0f);
    drawInsetCard (outputCard, 20.0f);

    const auto hintsCard = offsetHintLabel_.getBounds().getUnion (polarityHintLabel_.getBounds()).expanded (12, 12);
    drawInsetCard (hintsCard, 18.0f);

    const auto sidechainSnapshot = processor_.getAnalysisSnapshot();
    auto chipBounds = sidechainStatusLabel_.getBounds().expanded (12, 8).toFloat();
    chipBounds.setWidth (juce::jmin (chipBounds.getWidth(), analysisPanelBounds_.getWidth() - 48.0f));
    auto chipColour = sidechainSnapshot.sidechainConnected ? accentGreen : accentOrange;
    g.setColour (chipColour.withAlpha (0.62f));
    g.fillRoundedRectangle (chipBounds, 14.0f);
    g.setColour (chipColour.withAlpha (0.88f));
    g.drawRoundedRectangle (chipBounds, 14.0f, 1.0f);
}

void KickBassAssistantAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (28);
    auto header = bounds.removeFromTop (82);
    bounds.removeFromTop (12);
    auto helpArea = bounds.removeFromBottom (110);
    bounds.removeFromBottom (12);
    auto content = bounds;

    auto leftPanel = content.removeFromLeft (juce::roundToInt (content.getWidth() * 0.56f));
    content.removeFromLeft (12);
    auto rightPanel = content;

    headerPanelBounds_ = header.toFloat();
    controlsPanelBounds_ = leftPanel.toFloat();
    analysisPanelBounds_ = rightPanel.toFloat();
    helpPanelBounds_ = helpArea.toFloat();

    auto headerInner = header.reduced (24, 16);
    auto headerTop = headerInner.removeFromTop (30);
    auto languageArea = headerTop.removeFromRight (218);
    languageLabel_.setBounds (languageArea.removeFromLeft (80));
    languageBox_.setBounds (languageArea.reduced (0, 1));
    titleLabel_.setBounds (headerTop);
    headerInner.removeFromTop (2);
    subtitleLabel_.setBounds (headerInner.removeFromTop (22));

    auto left = leftPanel.reduced (22, 18);
    controlSectionLabel_.setBounds (left.removeFromTop (20));
    left.removeFromTop (12);

    auto controlRow = left.removeFromTop (46);
    listenLabel_.setBounds (controlRow.removeFromLeft (66));
    listenBox_.setBounds (controlRow.removeFromLeft (176));
    controlRow.removeFromLeft (10);
    polarityButton_.setBounds (controlRow.removeFromLeft (210));

    left.removeFromTop (16);
    auto sliderCardsArea = left;

    auto placeSliderCard = [] (juce::Slider& slider, juce::Label& label, juce::Rectangle<int> area)
    {
        auto inner = area.reduced (14, 12);
        label.setBounds (inner.removeFromTop (18));
        inner.removeFromTop (8);
        slider.setBounds (inner.removeFromTop (34));
    };

    auto splitTwoColumns = [] (juce::Rectangle<int> row, int gap)
    {
        std::array<juce::Rectangle<int>, 2> columns;
        const auto width = (row.getWidth() - gap) / 2;
        columns[0] = row.removeFromLeft (width);
        row.removeFromLeft (gap);
        columns[1] = row;
        return columns;
    };

    constexpr int cardGap = 12;
    constexpr int cardHeight = 82;
    auto firstRow = splitTwoColumns (sliderCardsArea.removeFromTop (cardHeight), cardGap);
    sliderCardsArea.removeFromTop (cardGap);
    auto secondRow = splitTwoColumns (sliderCardsArea.removeFromTop (cardHeight), cardGap);
    sliderCardsArea.removeFromTop (cardGap);
    auto thirdRow = splitTwoColumns (sliderCardsArea.removeFromTop (cardHeight), cardGap);

    placeSliderCard (offsetSlider_, offsetLabel_, firstRow[0]);
    placeSliderCard (crossoverSlider_, crossoverLabel_, firstRow[1]);
    placeSliderCard (duckSlider_, duckLabel_, secondRow[0]);
    placeSliderCard (attackSlider_, attackLabel_, secondRow[1]);
    placeSliderCard (releaseSlider_, releaseLabel_, thirdRow[0]);
    placeSliderCard (outputTrimSlider_, outputTrimLabel_, thirdRow[1]);

    auto right = rightPanel.reduced (22, 18);
    analysisSectionLabel_.setBounds (right.removeFromTop (20));
    right.removeFromTop (12);
    sidechainStatusLabel_.setBounds (right.removeFromTop (26));
    right.removeFromTop (14);

    constexpr int meterHeight = 52;
    conflictMeter_.setBounds (right.removeFromTop (meterHeight));
    right.removeFromTop (8);
    monoMeter_.setBounds (right.removeFromTop (meterHeight));
    right.removeFromTop (8);
    correlationMeter_.setBounds (right.removeFromTop (meterHeight));
    right.removeFromTop (8);
    detectorMeter_.setBounds (right.removeFromTop (meterHeight));
    right.removeFromTop (16);
    offsetHintLabel_.setBounds (right.removeFromTop (20));
    right.removeFromTop (8);
    polarityHintLabel_.setBounds (right.removeFromTop (20));

    auto helpInner = helpArea.reduced (22, 18);
    helpSectionLabel_.setBounds (helpInner.removeFromTop (20));
    helpInner.removeFromTop (10);
    helpTextLabel_.setBounds (helpInner);
}

void KickBassAssistantAudioProcessorEditor::timerCallback()
{
    updateLanguageStrings();

    const auto snapshot = processor_.getAnalysisSnapshot();
    const auto language = toUiLanguage (currentLanguageIndex_);

    conflictMeter_.setValue (snapshot.conflict);
    monoMeter_.setValue (snapshot.monoCompatibility);
    correlationMeter_.setValue (snapshot.lowEndCorrelation);
    detectorMeter_.setValue (snapshot.detector);

    sidechainStatusLabel_.setText (snapshot.sidechainConnected
                                       ? tr (language, "Sidechain: kick input detected", "侧链：已检测到 kick 输入")
                                       : tr (language, "Sidechain: waiting for kick input", "侧链：等待 kick 输入"),
                                   juce::dontSendNotification);

    offsetHintLabel_.setText (tr (language, "Alignment hint: set Offset to ", "对齐建议：将 Offset 设为 ")
                                  + juce::String (snapshot.recommendedOffsetMs, 2)
                                  + " ms",
                              juce::dontSendNotification);

    polarityHintLabel_.setText (snapshot.suggestInvertPolarity
                                    ? tr (language, "Polarity hint: invert low band", "极性建议：翻转低频")
                                    : tr (language, "Polarity hint: keep current polarity", "极性建议：保持当前极性"),
                                juce::dontSendNotification);
}

void KickBassAssistantAudioProcessorEditor::syncAnalysisLabels()
{
    timerCallback();
}
