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
BufferGlitchAudioProcessor::BufferGlitchAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    currentMode = Mode::BYPASS;
    
    addParameter (freezeMode = new AudioParameterBool ("freeze", "Freeze", false));
    addParameter (bufferSize = new AudioParameterInt ("blocksize", "Block size", 16, maxBufferSize, 1024));
    addParameter (glitchAmount = new AudioParameterFloat ("cpuoverload", "CPU overload", 0., 1., 0.));
}

BufferGlitchAudioProcessor::~BufferGlitchAudioProcessor()
{
}

//==============================================================================
const String BufferGlitchAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BufferGlitchAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BufferGlitchAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double BufferGlitchAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BufferGlitchAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BufferGlitchAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BufferGlitchAudioProcessor::setCurrentProgram (int index)
{
}

const String BufferGlitchAudioProcessor::getProgramName (int index)
{
    return {};
}

void BufferGlitchAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void BufferGlitchAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    frozenBuffer.setSize(getTotalNumOutputChannels(), maxBufferSize);
}

void BufferGlitchAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BufferGlitchAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BufferGlitchAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused channels
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // Freeze
    switch (currentMode)
    {
        case Mode::BYPASS:
            // Force freeze mode if Freeze param was changed with host
            if (*freezeMode)
                currentMode = Mode::FREEZE;
            else
            {
                // CPU overload can still happen in bypass...
                int destPosition = 0;
                
                // Iterate through the destination buffer
                while (destPosition < buffer.getNumSamples()) {
                    const int userBufferSize = *bufferSize;
                        
                    if (bufferReadPosition >= userBufferSize)
                    {
                        bufferReadPosition = 0;
                            
                        // We decide here if the next emulated buffer is "muted"
                        const float pick = Random::getSystemRandom().nextFloat();
                        mutedBuffer = (pick < *glitchAmount);
                    }
                        
                    const int remainingInSource = userBufferSize - bufferReadPosition;
                    const int remainingInDest = buffer.getNumSamples() - destPosition;
                        
                    const int numThisTime = jmin (remainingInSource, remainingInDest);
                        
                    if (mutedBuffer)
                        buffer.clear(destPosition, numThisTime);
                        
                    destPosition += numThisTime;
                    bufferReadPosition += numThisTime;
                }
            }
            
            break;
            
        case Mode::FREEZE:
        {
            frozenBuffer.clear();
            bufferWritePosition = 0;
            bufferReadPosition = 0;
            
            // We decide here if the first emulated buffer is "muted"
            const float pick = Random::getSystemRandom().nextFloat();
            mutedBuffer = (pick < *glitchAmount);
            
            currentMode = Mode::CAPTURE;
        }
        case Mode::CAPTURE:
        {
            const int numToWrite = maxBufferSize - bufferWritePosition;
            
            if (numToWrite <= 0) { currentMode = Mode::FROZEN; }
            else
            {
                const int numSourceSamples = buffer.getNumSamples();
                const int numThisTime = jmin(numToWrite, numSourceSamples);
                
                for (int c = 0; c < totalNumOutputChannels; c++)
                    frozenBuffer.addFrom(c, bufferWritePosition, buffer, c, 0, numThisTime);
                    
                bufferWritePosition += numThisTime;
                break;
            }
        }
        case Mode::FROZEN:
            // Force the exit of freeze mode if Freeze param was changed with host
            if (!*freezeMode)
            {
                currentMode = Mode::BYPASS;
                break;
            }
            
            for (int c = 0; c < totalNumOutputChannels; c++)
            {
                buffer.clear (c, 0, buffer.getNumSamples());
                
                int destPosition = 0;
                
                while (destPosition < buffer.getNumSamples()) {
                    const int userBufferSize = *bufferSize;
                    
                    if (bufferReadPosition >= userBufferSize)
                    {
                        bufferReadPosition = 0;
                        
                        // We decide here if the next emulated buffer is "muted"
                        const float pick = Random::getSystemRandom().nextFloat();
                        mutedBuffer = (pick < *glitchAmount);
                    }
                    
                    const int remainingInSource = userBufferSize - bufferReadPosition;
                    const int remainingInDest = buffer.getNumSamples() - destPosition;
                    
                    const int numThisTime = jmin (remainingInSource, remainingInDest);
                    
                    
                    
                    bool isHostPlaying = true;
                    
                    AudioPlayHead::CurrentPositionInfo info;
                    if (auto playHead = getPlayHead())
                    {
                        playHead->getCurrentPosition(info);
                        isHostPlaying = info.isPlaying;
                    }
                    
                    if (!mutedBuffer && isHostPlaying)
                        buffer.addFrom(c, destPosition, frozenBuffer, c, bufferReadPosition, numThisTime);
                    
                    destPosition += numThisTime;
                    bufferReadPosition += numThisTime;
                }
            }
            
            break;
    }
}

//==============================================================================
bool BufferGlitchAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BufferGlitchAudioProcessor::createEditor()
{
    return new BufferGlitchAudioProcessorEditor (*this);
}

//==============================================================================
void BufferGlitchAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BufferGlitchAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BufferGlitchAudioProcessor();
}
