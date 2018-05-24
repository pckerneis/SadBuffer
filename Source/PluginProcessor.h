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

#include "../JuceLibraryCode/JuceHeader.h"


//==============================================================================
/**
*/
class BufferGlitchAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    enum Mode {
        BYPASS,
        FREEZE,
        FROZEN,
        CAPTURE
    };
    
    //==============================================================================
    const static int maxBufferSize = 4096;
        
    //==============================================================================
    BufferGlitchAudioProcessor();
    ~BufferGlitchAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    bool isFrozen() const { return currentMode != Mode::BYPASS; }
    void freeze() { currentMode = Mode::FREEZE; *freezeMode = true; }
    void unfreeze() { currentMode = Mode::BYPASS; *freezeMode = false;}
    
    //==============================================================================
    void setBufferSize (int newSize) { *bufferSize = newSize; }
    void setGlitchAmount (float newAmount) { *glitchAmount = newAmount; }
    
private:
    friend class BufferGlitchAudioProcessorEditor;
    
    //==============================================================================
    AudioSampleBuffer frozenBuffer;
    Mode currentMode;
    
    AudioParameterInt* bufferSize;
    AudioParameterFloat* glitchAmount;
    AudioParameterBool* freezeMode;
    
    int bufferWritePosition = 0;
    int bufferReadPosition = 0;
    bool mutedBuffer = false;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferGlitchAudioProcessor)
};
