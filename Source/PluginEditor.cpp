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
    
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.333);
    
    auto responseWidth = responseArea.getWidth();
    
    auto& lowCut = monoChain.get<ChainPosition::LowCut>();
    auto& peak = monoChain.get<ChainPosition::Peak>();
    auto& highCut = monoChain.get<ChainPosition::HighCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    // 저장할 공간이 필요(double)
    std::vector<double> mags;
    
    mags.resize(responseWidth);
    
    for( int i = 0; i < responseWidth; ++i)
    {
        // 픽셀을 따라 반복하면서 주파수 진폭(Magnitude)을 계산
        // magnitude는 데시벨과 다르게 곱의 연산을 하므로 시작값 1을 주어야 함
        double mag = 1.f;
        
        // mapToLog10을 통해 픽셀 공간에 주파수를 매핑할 수 있다.
        auto freq = juce::mapToLog10(double(i) / double(responseWidth), 20.0, 20000.0);
        if (! monoChain.isBypassed<ChainPosition::Peak>() )
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (! lowCut.isBypassed<0>() )
            mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! lowCut.isBypassed<1>() )
            mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! lowCut.isBypassed<2>() )
            mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! lowCut.isBypassed<3>() )
            mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (! highCut.isBypassed<0>() )
            mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! highCut.isBypassed<1>() )
            mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! highCut.isBypassed<2>() )
            mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (! highCut.isBypassed<3>() )
            mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        mags[i] = juce::Decibels::gainToDecibels(mag);
    }
    
    juce::Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    
    auto map = [outputMin, outputMax](double input)
    {
        // window에 -24~24 범위의 데시벨 높이를 가지도록
        return juce::jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    // getX()->leftedge에서 시작, mags.front의 값 => 시작점
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    
    for( size_t i = 1; i < mags.size(); ++i )
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }
    
    g.setColour(juce::Colours::bisque);
    g.drawRect(responseArea.toFloat());
    
    g.setColour(juce::Colours::rosybrown);
    g.strokePath(responseCurve, juce::PathStrokeType(2.f));
}

void NormalEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    float getHeight = juce::Component::getHeight() / 3 * 2;
    float getWidth = juce::Component::getWidth() / 3 - 70;
   
    
    highCutFreqBox.setBounds(getWidth * 3, getHeight, 70, 25);
    peakFreqBox.setBounds(getWidth * 2, getHeight, 70, 25);
    peakGainBox.setBounds(getWidth * 2, getHeight * 1.333, 70, 25);
    peakQualityBox.setBounds(getWidth * 2, getHeight * 0.666, 70, 25);
    lowCutFreqBox.setBounds(getWidth * 1, getHeight, 70, 25);
}

void NormalEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
    // 파라미터가 변경되었을 때 atomic을 true로 바꿔줌
    parameterChanged.set(true);
}

void NormalEQAudioProcessorEditor::timerCallback()
{
    // true일 경우 다시 false로 바꾸기
    if( parameterChanged.compareAndSetBool(false,true) )
    {
        // monochain 업데이트
        // repaint를 통해 reponse curve 업데이트
        // 현재 aptvs monochain이 private이므로 리팩토링 필요
    }
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
