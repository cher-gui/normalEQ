/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
NormalEQAudioProcessorEditor::NormalEQAudioProcessorEditor (NormalEQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),


highCutFreqBox(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
peakFreqBox(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
peakGainBox(*audioProcessor.apvts.getParameter("Peak Gain"), "db"),
peakQualityBox(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
lowCutFreqBox(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),

highCutFreqBoxAttatchment(audioProcessor.apvts, "HighCut Freq", highCutFreqBox),
peakFreqBoxAttatchment(audioProcessor.apvts, "Peak Freq", peakFreqBox),
peakGainBoxAttatchment(audioProcessor.apvts, "Peak Gain", peakGainBox),
peakQualityBoxAttatchment(audioProcessor.apvts, "Peak Quality", peakQualityBox),
lowCutFreqBoxAttatchment(audioProcessor.apvts, "LowCut Freq", lowCutFreqBox),

highCutSlopeSliderAttatchment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider),
lowCutSlopeSliderAttatchment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider)

{
    setSize(700, 600);
    setWantsKeyboardFocus(true);
    
    for ( auto comp : getComps() )
    {
        addAndMakeVisible(comp);
        comp->setColour(juce::Slider::textBoxTextColourId, almond);
        comp->setColour(juce::Slider::textBoxOutlineColourId, almond);
        
    }
}

NormalEQAudioProcessorEditor::~NormalEQAudioProcessorEditor()
{
    
}

//==============================================================================
void NormalEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(background);

}

void NormalEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.333);
    float getHeight = juce::Component::getHeight() / 3 * 2;
    float getWidth = juce::Component::getWidth() / 3 - 70;
   
    
    highCutFreqBox.setBounds(getWidth * 3, getHeight, 70, 25);
    peakFreqBox.setBounds(getWidth * 2, getHeight, 70, 25);
    peakGainBox.setBounds(getWidth * 2, getHeight * 1.333, 70, 25);
    peakQualityBox.setBounds(getWidth * 2, getHeight * 0.666, 70, 25);
    lowCutFreqBox.setBounds(getWidth * 1, getHeight, 70, 25);
}

std::vector<juce::Component*> NormalEQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqBox,
        &peakGainBox,
        &peakQualityBox,
        &lowCutFreqBox,
        &highCutFreqBox,
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };
}
