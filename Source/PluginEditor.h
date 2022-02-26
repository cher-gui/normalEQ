/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "AbletonStyleBox.h"

struct CustomColour
{
    const juce::Colour background   = juce::Colour::fromRGB(12, 22, 49);
    const juce::Colour almondAlpha  = juce::Colour::fromRGBA(236, 216, 200, 100);
    const juce::Colour almond       = juce::Colour::fromRGB(236, 216, 200);
    const juce::Colour zest         = juce::Colour::fromRGB(218, 121, 25);
    const juce::Colour mahogany     = juce::Colour::fromRGB(97, 8, 7);
};

struct DrawResponseCurve : juce::Component,
                           juce::AudioProcessorParameter::Listener,
                           juce::Timer
{
    DrawResponseCurve(NormalEQAudioProcessor& p);
    ~DrawResponseCurve();

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateChain();

private:
    NormalEQAudioProcessor& audioProcessor;
    CustomColour customColour;
    juce::Atomic<bool> parameterChanged{ false };
    MonoChain  monoChain;
    
    juce::Image background;
    
    juce::Rectangle<int> getRenderArea();
    juce::Rectangle<int> getAnalysisArea();
};


struct CustomDialLookAndFeel : public juce::LookAndFeel_V4
{
    CustomDialLookAndFeel();
    ~CustomDialLookAndFeel();
    
    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override;
    void drawRotarySlider (juce::Graphics&, int x, int y,
                           int width, int height, float sliderPosProportional,
                           float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;
    
    juce::Label* createSliderTextBox (juce::Slider& slider) override;
    
private:
    CustomColour customColour;
};


struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider();
    ~CustomRotarySlider();

private:
    CustomColour customColour;
    CustomDialLookAndFeel customDialLookAndFeel;
};


struct DrawImage : public juce::Component
{
    juce::Image lowCutImage = juce::ImageCache::getFromMemory(BinaryData::highpass_png, BinaryData::highpass_pngSize);
    juce::Image peakImage = juce::ImageCache::getFromMemory(BinaryData::bell_png, BinaryData::bell_pngSize);
    juce::Image highCutImage = juce::ImageCache::getFromMemory(BinaryData::lowpass_png, BinaryData::lowpass_pngSize);
    juce::Image catImage = juce::ImageCache::getFromMemory(BinaryData::cat_png, BinaryData::cat_pngSize);
    
};

//==============================================================================
/**
*/

// 파라미터 변화에 응답하는 가장 쉬운 방법은 리스너로 등록하는 것
// 대부분의 콜백이 오디오 스레드에서 일어나며. 이는 우리가 콜백에서 editor의 filterChain을 업데이트하는 것과 같은 \
// gui 작업을 할 수 없다는 뜻
// 하지만 atomic flag와 같은 타이머를 설정할 수 있으며, 그를 기반으로 업데이트할 수 있다.

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
    CustomColour customColour;
    DrawImage drawImage;
    CustomLookAndFeel customLookAndFeel;
    
    AbletonStyleBox highCutFreqBox,
                    peakFreqBox,
                    peakGainBox,
                    peakQualityBox,
                    lowCutFreqBox;
    
    CustomRotarySlider highCutSlopeSlider,
                       lowCutSlopeSlider;
    
    DrawResponseCurve drawResponseCurveComponent;
    
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
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NormalEQAudioProcessorEditor)
};
