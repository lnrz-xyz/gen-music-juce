/*
  ==============================================================================

    Chord.h
    Created: 25 Jan 2024 12:54:36pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once

#include "Note.h"

class Note;

class Chord {
public:
    
    Chord(std::vector<Note> notes);
    
    void addNote(const Note& note) {
        notes.push_back(note);
    }
    
    void addNotes(std::vector<Note> notes);
    
    juce::MidiMessageSequence toMidiSequence(double bpm, double sampleRate) const;
    
private:
    std::vector<Note> notes;
};
