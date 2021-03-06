/*
  ==============================================================================
 
     This file is part of Sad Buffer.
     
     Sad Buffer is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.
     
     Sad Buffer is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with Sad Buffer.  If not, see <http://www.gnu.org/licenses/>.
 
  ==============================================================================
 
     Author:  Pierre-Clément KERNEIS
 
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BufferGlitchAudioProcessorEditor::BufferGlitchAudioProcessorEditor (BufferGlitchAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (250, 150);
    
    const auto dark = Colours::darkgrey;
    const auto light = Colours::grey;
    
    setLookAndFeel(&lf);
        
    freezeButton.setButtonText ("Freeze");
    freezeButton.setClickingTogglesState (true);
    freezeButton.addListener (this);
    freezeButton.setConnectedEdges(Button::ConnectedOnRight);
    freezeButton.setColour(TextButton::buttonColourId, dark);
    freezeButton.setColour(TextButton::buttonOnColourId, light);
    addAndMakeVisible(freezeButton);
    
    captureButton.setButtonText ("Capture");
    captureButton.setClickingTogglesState (false);
    captureButton.addListener (this);
    captureButton.setEnabled (false);
    captureButton.setConnectedEdges (Button::ConnectedOnLeft);
    captureButton.setColour (TextButton::buttonColourId, dark);
    addAndMakeVisible(captureButton);
    
    bufferSizeSlider.setSliderStyle (Slider::LinearBar);
    bufferSizeSlider.setRange (32, p.maxBufferSize, 1.0);
    bufferSizeSlider.setTextValueSuffix (" samples");
    bufferSizeSlider.setValue (1024, dontSendNotification);
    bufferSizeSlider.addListener( this);
    bufferSizeSlider.setColour (Slider::trackColourId, light.withAlpha(0.75f));
    addAndMakeVisible(bufferSizeSlider);

    glitchAmountSlider.setSliderStyle(Slider::LinearBar);
    glitchAmountSlider.setRange (0, 1, 0.001);
    glitchAmountSlider.setTextBoxStyle(Slider::TextBoxLeft, false, 100, 30);
    glitchAmountSlider.setValue(0., dontSendNotification);
    glitchAmountSlider.addListener(this);
    glitchAmountSlider.setColour(Slider::trackColourId, light.withAlpha(0.75f));
    addAndMakeVisible(glitchAmountSlider);
    
    cpuLabel.setText("CPU overload", dontSendNotification);
    cpuLabel.setLookAndFeel(&lf);
    addAndMakeVisible(cpuLabel);
    
    bufferSizeLabel.setText("Block size", dontSendNotification);
    addAndMakeVisible(bufferSizeLabel);
    
    bgImage = ImageFileFormat::loadFrom (BinaryData::sad_gif, (size_t) BinaryData::sad_gifSize);
    
    infoButton.setButtonText ("?");
    infoButton.setClickingTogglesState (true);
    infoButton.addListener (this);
    infoButton.setLookAndFeel (&lf);
    infoButton.setColour (TextButton::buttonColourId, dark);
    infoButton.setColour (TextButton::buttonOnColourId, dark);
    infoButton.setColour (TextButton::textColourOnId, light);
    infoButton.setColour (TextButton::textColourOffId, light);
    addAndMakeVisible(infoButton);
    
    startTimer (100);
}

BufferGlitchAudioProcessorEditor::~BufferGlitchAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void BufferGlitchAudioProcessorEditor::paint (Graphics& g)
{
    g.drawImageAt (bgImage, 0, 0);
    g.setFont (lf.getDefaultFont());
    g.setColour (Colours::white);
    g.drawText ("sad_buffer", getLocalBounds().removeFromTop(40), Justification::centred);
    
    const bool infoShown = infoButton.getToggleState();
    
    if (infoShown)
    {
        const String version = "1.1.0";
        const String text = "v." + version + " made with JUCE by pc.kerneis";
        
        g.setColour(Colours::lightgrey);
        g.setFont(lf.getDefaultFont().withHeight(10));
        g.drawText(text, getLocalBounds().removeFromBottom(20).withTrimmedRight(24), Justification::right);
    }
}

void BufferGlitchAudioProcessorEditor::resized()
{
    auto r = getLocalBounds().reduced (30);
    r.removeFromTop (10);
    
    const int itemWidth = r.proportionOfWidth(0.5f);
    const int rowHeight = 25;
    
    auto row1 = r.removeFromTop (rowHeight);
    freezeButton.setBounds (row1.removeFromLeft(itemWidth));
    captureButton.setBounds (row1);
    
    auto row2 = r.removeFromTop (rowHeight);
    bufferSizeLabel.setBounds (row2.removeFromLeft(itemWidth));
    bufferSizeSlider.setBounds (row2);
    
    auto row3 = r.removeFromTop (rowHeight);
    cpuLabel.setBounds (row3.removeFromLeft(itemWidth));
    glitchAmountSlider.setBounds (row3);
    
    infoButton.setBounds(getLocalBounds().removeFromBottom(20).removeFromRight(20));
}

void BufferGlitchAudioProcessorEditor::buttonClicked(Button* b)
{
    if (b == &freezeButton)
        toggleFreezeState();
    if (b == &captureButton)
        processor.freeze();
    if (b == &infoButton)
        toggleInfoState();
}

void BufferGlitchAudioProcessorEditor::sliderValueChanged(Slider* s)
{
    if (s == &bufferSizeSlider)
        processor.setBufferSize(s->getValue());
    
    else if (s == &glitchAmountSlider)
        processor.setGlitchAmount(s->getValue());
}

void BufferGlitchAudioProcessorEditor::editorShown (Label* label, TextEditor& editor)
{
    Font editorFont (lf.getDefaultFont());
    editorFont.setItalic (true);
    editor.setFont (editorFont);
}

void BufferGlitchAudioProcessorEditor::timerCallback()
{
    const OwnedArray<AudioProcessorParameter>& params = getAudioProcessor()->getParameters();
    for (int i = 0; i < params.size(); ++i)
    {
        if (const AudioParameterFloat* param = dynamic_cast<AudioParameterFloat*> (params[i]))
        {
            if (param == processor.glitchAmount)
                glitchAmountSlider.setValue (*param, dontSendNotification);
        }
        else if (const AudioParameterInt* param = dynamic_cast<AudioParameterInt*> (params[i]))
        {
            if (param == processor.bufferSize)
                bufferSizeSlider.setValue (*param, dontSendNotification);
        }
        else if (const AudioParameterBool* param = dynamic_cast<AudioParameterBool*> (params[i]))
            if (param == processor.freezeMode)
                setFreezeButtonState(*param);
    }
}

void BufferGlitchAudioProcessorEditor::toggleFreezeState()
{
    const bool isFrozen = processor.isFrozen();
    const String text = isFrozen ? "Un-freeze" : "Freeze";
    
    freezeButton.setButtonText (text);
    captureButton.setEnabled(!isFrozen);
    
    if (isFrozen)
        processor.unfreeze();
    else
        processor.freeze();
}

void BufferGlitchAudioProcessorEditor::setFreezeButtonState (bool shouldBeOn)
{
    const String text = shouldBeOn ? "Un-freeze" : "Freeze";
    
    freezeButton.setToggleState (shouldBeOn, dontSendNotification);
    freezeButton.setButtonText (text);
    captureButton.setEnabled (shouldBeOn);
}

void BufferGlitchAudioProcessorEditor::toggleInfoState()
{
    const bool infoShown = infoButton.getToggleState();
        
    const String text = infoShown ? "x" : "?";
    infoButton.setButtonText (text);
    
    repaint();
}
