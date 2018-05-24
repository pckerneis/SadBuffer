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

#pragma once

#include "PluginProcessor.h"

#include "LookAndFeel.h"

//==============================================================================
class BufferGlitchAudioProcessorEditor
    :   public AudioProcessorEditor,
        public Button::Listener,
        public Slider::Listener,
        public Label::Listener,
        public Timer
{
public:
    BufferGlitchAudioProcessorEditor (BufferGlitchAudioProcessor&);
    ~BufferGlitchAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void buttonClicked(Button* b) override;
    void sliderValueChanged(Slider* s) override;
    
    void editorShown (Label* label, TextEditor& editor) override;    
    void labelTextChanged (Label* labelThatHasChanged) override {}
    void editorHidden (Label*, TextEditor&) override {}
    
    //==============================================================================
    void timerCallback() override;

private:
    void toggleFreezeState();
    void setFreezeButtonState (bool shouldBeOn);
    
    void toggleInfoState();
    
    //==============================================================================
    BufferGlitchAudioProcessor& processor;
    
    TextButton freezeButton;
    TextButton captureButton;
    Slider bufferSizeSlider;
    Slider glitchAmountSlider;
    
    TextButton infoButton;
    
    Label cpuLabel;
    Label bufferSizeLabel;
    
    Image bgImage;
    
    CustomLookAndFeel lf;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferGlitchAudioProcessorEditor)
};
