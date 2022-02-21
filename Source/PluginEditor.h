/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "AbletonStyleBox.h"


struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(RotaryVerticalDrag, NoTextBox)
    {
    
    }
};

//==============================================================================
/**
*/
class NormalEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NormalEQAudioProcessorEditor (NormalEQAudioProcessor&);
    ~NormalEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NormalEQAudioProcessor& audioProcessor;
    
    AbletonStyleBox highCutFreqBox,
                    peakFreqBox,
                    peakGainBox,
                    peakQualityBox,
                    lowCutFreqBox;
    
    CustomRotarySlider highCutSlopeSlider,
    lowCutSlopeSlider;
    
    
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attatchment = APVTS::SliderAttachment;
    
    Attatchment highCutFreqBoxAttatchment,
                peakFreqBoxAttatchment,
                peakGainBoxAttatchment,
                peakQualityBoxAttatchment,
                lowCutFreqBoxAttatchment,
                highCutSlopeSliderAttatchment,
                lowCutSlopeSliderAttatchment;
    
 
    
    
    std::vector<juce::Component*> getComps();
    
    juce::Colour background = juce::Colour::fromRGB(12, 22, 49);
    juce::Colour almond = juce::Colour::fromRGB(236, 216, 200);
    juce::Colour zest = juce::Colour::fromRGB(218, 121, 25);
    juce::Colour mahogany = juce::Colour::fromRGB(97,8,7);
    
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NormalEQAudioProcessorEditor)
};
