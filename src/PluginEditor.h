#pragma once

#include "PluginProcessor.h"

class KickBassAssistantAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                    private juce::Timer
{
public:
    explicit KickBassAssistantAudioProcessorEditor (KickBassAssistantAudioProcessor&);
    ~KickBassAssistantAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class ModernLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        ModernLookAndFeel();

        void drawRotarySlider (juce::Graphics& g,
                               int x,
                               int y,
                               int width,
                               int height,
                               float sliderPosProportional,
                               float rotaryStartAngle,
                               float rotaryEndAngle,
                               juce::Slider& slider) override;

        void drawLinearSlider (juce::Graphics& g,
                               int x,
                               int y,
                               int width,
                               int height,
                               float sliderPos,
                               float minSliderPos,
                               float maxSliderPos,
                               const juce::Slider::SliderStyle style,
                               juce::Slider& slider) override;

        void drawComboBox (juce::Graphics& g,
                           int width,
                           int height,
                           bool isButtonDown,
                           int buttonX,
                           int buttonY,
                           int buttonW,
                           int buttonH,
                           juce::ComboBox& box) override;

        juce::Font getComboBoxFont (juce::ComboBox& box) override;
        void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;
        void drawButtonBackground (juce::Graphics& g,
                                   juce::Button& button,
                                   const juce::Colour& backgroundColour,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown) override;
        juce::Font getTextButtonFont (juce::TextButton& button, int buttonHeight) override;
        void drawButtonText (juce::Graphics& g,
                             juce::TextButton& button,
                             bool shouldDrawButtonAsHighlighted,
                             bool shouldDrawButtonAsDown) override;
    };

    class MeterComponent final : public juce::Component
    {
    public:
        MeterComponent (juce::String title, juce::Colour activeColour);

        void setTitle (juce::String newTitle);
        void setValue (float newValue);
        void paint (juce::Graphics& g) override;

    private:
        juce::String title_;
        juce::Colour activeColour_;
        float value_ = 0.0f;
    };

    static void styleSectionLabel (juce::Label& label, const juce::String& text);
    static void styleRotarySlider (juce::Slider& slider);

    void configureHelpEntries();
    void updateLanguageStrings();
    void registerHelpTarget (juce::Component& component, juce::String text);
    void updateHelpTextForComponent (const juce::Component* component);
    [[nodiscard]] juce::String findHelpTextForComponent (const juce::Component* component) const;

    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseMove (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;
    void timerCallback() override;
    void syncAnalysisLabels();

    KickBassAssistantAudioProcessor& processor_;

    juce::Label titleLabel_;
    juce::Label subtitleLabel_;
    juce::Label languageLabel_;
    juce::Label controlSectionLabel_;
    juce::Label analysisSectionLabel_;
    juce::Label helpSectionLabel_;
    juce::Label sidechainStatusLabel_;
    juce::Label offsetHintLabel_;
    juce::Label polarityHintLabel_;
    juce::Label helpTextLabel_;

    juce::ComboBox listenBox_;
    juce::ComboBox languageBox_;
    juce::TextButton polarityButton_ { "Flip Low Polarity" };
    juce::Slider offsetSlider_;
    juce::Slider crossoverSlider_;
    juce::Slider duckSlider_;
    juce::Slider attackSlider_;
    juce::Slider releaseSlider_;
    juce::Slider outputTrimSlider_;

    juce::Label listenLabel_ { {}, "Listen" };
    juce::Label offsetLabel_ { {}, "Offset" };
    juce::Label crossoverLabel_ { {}, "Crossover" };
    juce::Label duckLabel_ { {}, "Duck" };
    juce::Label attackLabel_ { {}, "Attack" };
    juce::Label releaseLabel_ { {}, "Release" };
    juce::Label outputTrimLabel_ { {}, "Output" };

    MeterComponent conflictMeter_ { "Conflict", juce::Colour::fromRGB (255, 69, 58) };
    MeterComponent monoMeter_ { "Mono", juce::Colour::fromRGB (50, 215, 75) };
    MeterComponent correlationMeter_ { "Correlation", juce::Colour::fromRGB (10, 132, 255) };
    MeterComponent detectorMeter_ { "Detector", juce::Colour::fromRGB (255, 159, 10) };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    ComboAttachment languageAttachment_;
    ComboAttachment listenAttachment_;
    ButtonAttachment polarityAttachment_;
    SliderAttachment offsetAttachment_;
    SliderAttachment crossoverAttachment_;
    SliderAttachment duckAttachment_;
    SliderAttachment attackAttachment_;
    SliderAttachment releaseAttachment_;
    SliderAttachment outputTrimAttachment_;

    std::atomic<float>* languageParam_ = nullptr;
    std::vector<std::pair<const juce::Component*, juce::String>> helpEntries_;
    juce::String defaultHelpText_;
    juce::Rectangle<float> headerPanelBounds_;
    juce::Rectangle<float> controlsPanelBounds_;
    juce::Rectangle<float> analysisPanelBounds_;
    juce::Rectangle<float> helpPanelBounds_;
    const juce::Component* activeHelpComponent_ = nullptr;
    int currentLanguageIndex_ = -1;
    ModernLookAndFeel lookAndFeel_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KickBassAssistantAudioProcessorEditor)
};
