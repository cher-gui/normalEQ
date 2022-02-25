/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


DrawResponseCurve::DrawResponseCurve(NormalEQAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }

    startTimerHz(60);

}

DrawResponseCurve::~DrawResponseCurve()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}

void DrawResponseCurve::paint(juce::Graphics& g)
{
    auto responseArea = getLocalBounds();
    auto responseWidth = responseArea.getWidth();

    auto& lowCut = monoChain.get<ChainPosition::LowCut>();
    auto& peak = monoChain.get<ChainPosition::Peak>();
    auto& highCut = monoChain.get<ChainPosition::HighCut>();

    auto sampleRate = audioProcessor.getSampleRate();

    // 저장할 공간이 필요(double)
    std::vector<double> mags;

    mags.resize(responseWidth);

    for (int i = 0; i < responseWidth; ++i)
    {
        // 픽셀을 따라 반복하면서 주파수 진폭(Magnitude)을 계산
        // magnitude는 데시벨과 다르게 곱의 연산을 하므로 시작값 1을 주어야 함
        double mag = 1.f;

        // mapToLog10을 통해 픽셀 공간에 주파수를 매핑할 수 있다.
        auto freq = juce::mapToLog10(double(i) / double(responseWidth), 20.0, 20000.0);
        if (!monoChain.isBypassed<ChainPosition::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!lowCut.isBypassed<0>())
            mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowCut.isBypassed<1>())
            mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowCut.isBypassed<2>())
            mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!lowCut.isBypassed<3>())
            mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        if (!highCut.isBypassed<0>())
            mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highCut.isBypassed<1>())
            mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highCut.isBypassed<2>())
            mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (!highCut.isBypassed<3>())
            mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);

        mags[i] = juce::Decibels::gainToDecibels(mag);
    }

    juce::Path responseCurve;

    const double outputMin = responseArea.getBottom() - 2;
    const double outputMax = responseArea.getY();

    auto map = [outputMin, outputMax](double input)
    {
        // window에 -24~24 범위의 데시벨 높이를 가지도록
        return juce::jmap(input, -24.0, 24.0, outputMin, outputMax);
    };

    // getX()->leftedge에서 시작, mags.front의 값 => 시작점
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));

    for (size_t i = 1; i < mags.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }

    g.setColour(customColour.almond);
    g.drawLine(0, responseArea.getBottom(), responseArea.getRight(), responseArea.getBottom(), 0.2f);

    g.setColour(customColour.almond);
    g.strokePath(responseCurve, juce::PathStrokeType(2.1f));
}

void DrawResponseCurve::parameterValueChanged(int parameterIndex, float newValue)
{
    // 파라미터가 변경되었을 때 atomic을 true로 바꿔줌
    parameterChanged.set(true);
}

void DrawResponseCurve::timerCallback()
{
    // true일 경우 다시 false로 바꾸기
    if (parameterChanged.compareAndSetBool(false, true))
    {
        // monochain 업데이트
        auto chainSettings = getChainSettings(audioProcessor.apvts);
        auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
        NormalEQAudioProcessor::updateCoefficients(monoChain.get<ChainPosition::Peak>().coefficients, peakCoefficients);

        // 타이머 업데이트가 필요함
        // repaint를 통해 reponse curve 업데이트
        repaint();
    }
}
//==============================================================================

CustomDialLookAndFeel::CustomDialLookAndFeel() {}
CustomDialLookAndFeel::~CustomDialLookAndFeel() {}

juce::Slider::SliderLayout CustomDialLookAndFeel::getSliderLayout(juce::Slider &slider)
{
    auto bounds = slider.getLocalBounds();
    
    juce::Slider::SliderLayout layout;
    layout.textBoxBounds = bounds.withY(0);
    layout.sliderBounds = bounds;
    
    return layout;
}

void CustomDialLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y,
                                             int width, int height,
                                             float sliderPos, const float rotaryStartAngle,
                                             const float rotaryEndAngle, juce::Slider& slider)
{

    auto bounds = juce::Rectangle<float> (x, y, width, height).reduced (2.0f);
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = radius * 0.035f;
    auto arcRadius = radius - lineW * 1.1f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(),
                                 bounds.getCentreY(),
                                 arcRadius,
                                 arcRadius,
                                 0.0f,
                                 rotaryStartAngle,
                                 rotaryEndAngle,
                                 true);

    g.setColour (customColour.almond);
    g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc (bounds.getCentreX(),
                            bounds.getCentreY(),
                            arcRadius,
                            arcRadius,
                            0.0f,
                            rotaryStartAngle,
                            toAngle,
                            true);

    g.setColour (customColour.almond);
    g.strokePath (valueArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path stick;
    
    auto stickWidth = lineW * 2.0f;
    g.setColour(customColour.almond);
    stick.addRectangle (-stickWidth / 2, -radius -1, 3.5, 3.5);
    g.fillPath (stick, juce::AffineTransform::rotation (toAngle).translated (bounds.getCentre()));
}

juce::Label* CustomDialLookAndFeel::createSliderTextBox(juce::Slider &slider)
{
    auto* l = new juce::Label();

    l->setJustificationType (juce::Justification::centred);
    l->setColour (juce::Label::textColourId, slider.findColour (juce::Slider::textBoxTextColourId));
    l->setColour (juce::Label::textWhenEditingColourId, slider.findColour (juce::Slider::textBoxTextColourId));
    l->setColour (juce::Label::outlineWhenEditingColourId, juce::Colours::transparentWhite);
    l->setInterceptsMouseClicks (false, false);
    l->setFont (18.0f);

    return l;
}

CustomRotarySlider::CustomRotarySlider() 
{
    setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    //setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 50);
    setRotaryParameters(juce::MathConstants<float>::pi * 1.75f, juce::MathConstants<float>::pi * 2.25f, true);
    

    
    //setColour(juce::Slider::textBoxTextColourId, customColour.almond);
    //setColour(juce::Slider::textBoxOutlineColourId, customColour.background);
    //setColour(juce::Slider::rotarySliderFillColourId, customColour.almond);

    setLookAndFeel(&customDialLookAndFeel);
    
    setVelocityBasedMode(true);
    setVelocityModeParameters(1.0, 1, 0.1, false);
    setRange(0, 3, 1);
    setValue(0);
    setDoubleClickReturnValue(false,false);
    setTextValueSuffix("");
    
}

CustomRotarySlider::~CustomRotarySlider()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
NormalEQAudioProcessorEditor::NormalEQAudioProcessorEditor(NormalEQAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),

    drawResponseCurveComponent(audioProcessor),
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
    setSize(650, 650);
    setWantsKeyboardFocus(true);
    
    for ( auto comp : getComps() )
    {
        addAndMakeVisible(comp);
        comp->setColour(juce::Slider::textBoxTextColourId, customColour.almond);
        comp->setColour(juce::Slider::textBoxOutlineColourId, customColour.almond); 
    }
    
    juce::LookAndFeel::setDefaultLookAndFeel(&customLookAndFeel);
    

}

NormalEQAudioProcessorEditor::~NormalEQAudioProcessorEditor()
{
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

//==============================================================================
void NormalEQAudioProcessorEditor::paint(juce::Graphics& g)
{
    
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(customColour.background);
    
    
    g.setColour(customColour.almond);
    auto leftCenter = peakFreqBox.getX() - (abs(peakFreqBox.getX()-lowCutFreqBox.getX()) / 2) + 35;
    g.drawLine(leftCenter, peakGainBox.getBottom(), leftCenter, peakQualityBox.getY(), 0.15f);
    
    g.setColour(customColour.almond);
    auto rightCenter = highCutFreqBox.getX() - (abs(peakFreqBox.getX()-highCutFreqBox.getX()) / 2) + 35;
    g.drawLine(rightCenter, peakGainBox.getBottom(), rightCenter, peakQualityBox.getY(), 0.15f);
    
    float getWidth = juce::Component::getWidth();
    float getHeight = juce::Component::getHeight();
    g.drawImage(drawImage.catImage, 0, 0, 50, 50, 0, 0, drawImage.catImage.getWidth(), drawImage.catImage.getHeight());
    g.drawImage(drawImage.lowCutImage, getWidth * 0.2 - 12, getHeight * 0.37, 24, 20, 0, 0, drawImage.lowCutImage.getWidth(),drawImage.lowCutImage.getHeight());
    g.drawImage(drawImage.peakImage, getWidth * 0.5 - 12, getHeight * 0.37, 24, 20, 0, 0, drawImage.peakImage.getWidth(),drawImage.peakImage.getHeight());
    g.drawImage(drawImage.highCutImage, getWidth * 0.8 - 12, getHeight * 0.37, 24, 20, 0, 0, drawImage.highCutImage.getWidth(),drawImage.highCutImage.getHeight());
} 

void NormalEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    float getHeight = juce::Component::getHeight() / 3 * 2 + 10;
    float getWidth = juce::Component::getWidth() / 3 - 70;
   
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.333);
    
    highCutFreqBox.setBounds(getWidth * 3, getHeight, 70, 25);
    peakFreqBox.setBounds(getWidth * 2, getHeight, 70, 25);
    peakGainBox.setBounds(getWidth * 2, getHeight * 1.3, 70, 25);
    peakQualityBox.setBounds(getWidth * 2, getHeight * 0.7, 70, 25);
    lowCutFreqBox.setBounds(getWidth * 1, getHeight, 70, 25);
    lowCutSlopeSlider.setBounds(getWidth * 0.5, getHeight * 0.98, 50, 50);
    highCutSlopeSlider.setBounds(getWidth * 3.5 + 25, getHeight * 0.98, 50, 50);
    drawResponseCurveComponent.setBounds(responseArea);
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
        &highCutSlopeSlider,
        &drawResponseCurveComponent
    };
}
