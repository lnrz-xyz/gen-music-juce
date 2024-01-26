/*
  ==============================================================================

    Track.cpp
    Created: 23 Jan 2024 2:41:46pm
    Author:  Benjamin Conn

  ==============================================================================
*/


#include "Track.h"
#include "Song.h"
#include "Note.h"
#include <JuceHeader.h>
#include "Utilities.h"

void Track::addNote(const Note& note) {
    juce::MidiMessageSequence sequence = note.toMidiSequence(song->getBPM(), song->getSampleRate());
    midiSequence.addSequence(sequence, 0);
}

void Track::addChord(const Chord& chord) {
    juce::MidiMessageSequence sequence = chord.toMidiSequence(song->getBPM(), song->getSampleRate());
    midiSequence.addSequence(sequence, 0);
}

void Track::render(juce::AudioBuffer<float>& buffer) {
    juce::MidiBuffer midiBuffer;
    for (int i = 0; i < midiSequence.getNumEvents(); ++i) {
        auto* midiEvent = midiSequence.getEventPointer(i);
        int sampleNumber = static_cast<int>(midiEvent->message.getTimeStamp()); // Convert time to samples
        midiBuffer.addEvent(midiEvent->message, sampleNumber);
    }

    synth->renderNextBlock(buffer, midiBuffer, 0, buffer.getNumSamples());
}

double Track::getTotalTime() {
    return 20.0;
}
