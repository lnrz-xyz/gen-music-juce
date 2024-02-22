/*
  ==============================================================================

    MIDIRenderer.cpp
    Created: 21 Feb 2024 11:51:09am
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "MIDIRenderer.h"
#include "Note.h"
#include <JuceHeader.h>


MIDIRenderer::MIDIRenderer(double bpm, double sampleRate) : bpm(bpm), sampleRate(sampleRate) {
    
}

juce::MidiMessageSequence MIDIRenderer::toMidiSequence(std::vector<Note> notes) {
    juce::MidiMessageSequence sequence;
    for (auto note : notes) {
        double startTimeInSeconds = note.startTimeInBeats * (60.0 / bpm);
        double durationInSeconds = note.durationInBeats * (60.0 / bpm);
        
        auto startSample = static_cast<int>(startTimeInSeconds * sampleRate);
        auto endSample = startSample + static_cast<int>(durationInSeconds * sampleRate);
        
        
        sequence.addEvent(juce::MidiMessage::noteOn(1, note.midiNoteNumber, note.velocity), startSample);
        sequence.addEvent(juce::MidiMessage::noteOff(1, note.midiNoteNumber), endSample);
    }
    return sequence;
}
