/*
  ==============================================================================

    AbletonStyleBox.cpp
    Created: 21 Feb 2022 5:53:03pm
    Author:  hc

  ==============================================================================
*/

#include "AbletonStyleBox.h"
CustomLookAndFeel::CustomLookAndFeel()
{
    auto font = juce::Typeface::createSystemTypefaceFor(BinaryData::ScopeOneRegular_ttf, BinaryData::ScopeOneRegular_ttfSize);
    setDefaultSansSerifTypeface(font);
}
CustomLookAndFeel::~CustomLookAndFeel(){};

juce::CaretComponent* CustomLookAndFeel::createCaretComponent(juce::Component *keyFocusOwner)
{
    auto caret = new juce::CaretComponent(keyFocusOwner);

    caret->setColour (juce::CaretComponent::caretColourId, keyFocusOwner->findColour(juce::Label::textColourId));

    return caret;
}

juce::Label* CustomLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* l = new juce::Label();
    
    l->setJustificationType(juce::Justification::centred);
    l->setColour(juce::Label::textColourId, slider.findColour(juce::Slider::textBoxTextColourId));
    l->setColour(juce::Label::textWhenEditingColourId, slider.findColour(juce::Slider::textBoxTextColourId));
    l->setColour(juce::Label::outlineWhenEditingColourId, juce::Colours::transparentWhite);
    
    l->setFont(20);
    

    return l;
}

AbletonStyleBox::AbletonStyleBox()
{

}
AbletonStyleBox::AbletonStyleBox(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) : param(&rap),suffix(unitSuffix)
{
    
    
    setTextValueSuffix(suffix);
    setLookAndFeel(&customLookAndFeel);
    setSliderStyle(juce::Slider::LinearBar);

    setColour(juce::Slider::trackColourId, juce::Colours::red);

    setTextBoxIsEditable(false);
    setVelocityBasedMode(true);
    //setVelocityModeParameters(0.5, 1, 0.09, false);
    //setRange(0, 100, 0.01);
    //setValue(50.0);

    //setDoubleClickReturnValue(true, 50.0);
    setWantsKeyboardFocus(true);
    
    /** You can assign a lambda to this callback object to have it called when the slider value is changed. */
    onValueChange = [&]()
    {
        if (getValue() < 10)
            setNumDecimalPlacesToDisplay(2);
        else if (10 <= getValue() && getValue() < 100)
            setNumDecimalPlacesToDisplay(1);
        else
            setNumDecimalPlacesToDisplay(0);
    };
    
}

AbletonStyleBox::~AbletonStyleBox()
{
    setLookAndFeel(nullptr);
}

void AbletonStyleBox::paint(juce::Graphics & g)
{
    if (hasKeyboardFocus (false))
    {
        auto bounds = getLocalBounds().toFloat();
        auto h = bounds.getHeight();
        auto w = bounds.getWidth();
        auto len = juce::jmin (h, w) * 0.17f;
        auto thick  = len / 1.8f;
        
        g.setColour (findColour (juce::Slider::textBoxOutlineColourId));
        // Left top
        g.drawRect(2.0f, 2.0f, 2.0f, 2.0f);
        g.drawLine (0.0f, 0.0f, 0.0f, len, thick);
        g.drawLine (0.0f, 0.0f, len, 0.0f, thick);

        // Right bottom
        /*
        g.drawRect(w-2.0f, h-2.0f, 2.0f, 2.0f);
        g.drawLine(w, h, w, h - len, thick);
        g.drawLine(w, h, w - len, h, thick);
        */
    }
}

void AbletonStyleBox::mouseDown (const juce::MouseEvent& event)
{
    juce::Slider::mouseDown (event);
    setMouseCursor (juce::MouseCursor::NoCursor);
}

void AbletonStyleBox::mouseUp (const juce::MouseEvent& event)
{
    juce::Slider::mouseUp (event);
    juce::Desktop::getInstance().getMainMouseSource().setScreenPosition (event.source.getLastMouseDownPosition());
    setMouseCursor (juce::MouseCursor::NormalCursor);
}


