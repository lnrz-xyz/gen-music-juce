/*
  ==============================================================================

    AudioRenderer.cpp
    Created: 21 Feb 2024 11:51:29am
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "AudioRenderer.h"
#include <fmt/core.h>

AudioRenderer::AudioRenderer(double sampleRate) : sampleRate(sampleRate) {}

void AudioRenderer::renderMIDISequence(juce::AudioBuffer<float>& buffer, juce::MidiMessageSequence* sequence, juce::Synthesiser* synth) {
    juce::MidiBuffer midiBuffer;
    fmt::println("Rendering MIDI sequence {}", sequence->getNumEvents());
    for (int i = 0; i < sequence->getNumEvents(); ++i) {
        auto* midiEvent = sequence->getEventPointer(i);
        int sampleNumber = static_cast<int>(midiEvent->message.getTimeStamp()); // Convert time to samples
        midiBuffer.addEvent(midiEvent->message, sampleNumber);
    }
    
    synth->renderNextBlock(buffer, midiBuffer, 0, buffer.getNumSamples());
}
