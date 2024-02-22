/*
  ==============================================================================

    AudioProcessingBus.cpp
    Created: 21 Feb 2024 11:51:44am
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "AudioProcessingBus.h"
#include <fmt/core.h>

AudioProcessingBus::AudioProcessingBus(double sampleRate) : renderer(sampleRate) {
    
}

void AudioProcessingBus::render(const std::vector<std::pair<juce::MidiMessageSequence*, juce::Synthesiser*>>& midiSynthPairs, juce::AudioBuffer<float>& outputBuffer, EffectProcessor* processor) {
    for (auto& pair : midiSynthPairs) {
        renderer.renderMIDISequence(outputBuffer, pair.first, pair.second);
    }
    
    fmt::println("Processing buffer with {} samples", outputBuffer.getNumSamples());
    
    processor->process(outputBuffer);
}
