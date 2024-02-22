/*
  ==============================================================================

    AudioProcessingBus.h
    Created: 21 Feb 2024 11:51:44am
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "EffectProcessor.h"
#include "AudioRenderer.h"

class AudioProcessingBus {
public:
    AudioProcessingBus(double sampleRate);
    
    void render(const std::vector<std::pair<juce::MidiMessageSequence*, juce::Synthesiser*>>& midiSynthPairs, juce::AudioBuffer<float>& outputBuffer, EffectProcessor* processor);
    
private:
    AudioRenderer renderer;
};
