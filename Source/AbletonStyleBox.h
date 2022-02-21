/*
  ==============================================================================

    AbletonStyleBox.h
    Created: 21 Feb 2022 5:53:03pm
    Author:  hc

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    
    juce::CaretComponent* createCaretComponent(juce::Component* KeyFocusOwner) override;
    juce::Label* createSliderTextBox(juce::Slider& slider) override;

};

class AbletonStyleBox : public juce::Slider
{
public:
    AbletonStyleBox();
    AbletonStyleBox(juce::RangedAudioParameter &rap, const juce::String &unitSuffix);
    ~AbletonStyleBox();
    
    void paint(juce::Graphics& g) override;
    
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
    
private:
    CustomLookAndFeel customLookAndFeel;
    
    juce::Slider::LookAndFeelMethods* lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};
