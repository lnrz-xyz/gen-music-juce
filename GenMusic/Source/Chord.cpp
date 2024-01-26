/*
  ==============================================================================

    Chord.cpp
    Created: 25 Jan 2024 12:59:19pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "Chord.h"
#include "Note.h"
#include "Utilities.h"


juce::MidiMessageSequence Chord::toMidiSequence(double bpm, double sampleRate) const {
    juce::MidiMessageSequence sequence;
    for (const auto& note : notes) {
        auto noteSequence = note.toMidiSequence(bpm,sampleRate);
        sequence.addSequence(noteSequence, 0, 0, noteSequence.getEndTime());
    }
    return sequence;
}


void Chord::addNotes(std::vector<Note> newNotes) {
    for (const auto& note : newNotes) {
        notes.push_back(note);
    }
}

Chord::Chord(std::vector<Note> newNotes) {
    addNotes(newNotes);
}
