/*
  ==============================================================================

    AudioRenderer.h
    Created: 21 Feb 2024 11:51:29am
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "MIDIRenderer.h"

class AudioRenderer {
public:
    
    AudioRenderer(double sampleRate);
    
    
    void renderMIDISequence(juce::AudioBuffer<float>& buffer, juce::MidiMessageSequence* sequence, juce::Synthesiser* synth);
    
private:
    double sampleRate;
};
