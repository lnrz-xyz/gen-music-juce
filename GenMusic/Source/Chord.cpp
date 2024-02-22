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


void Chord::addNotes(std::vector<Note> newNotes) {
    for (const auto& note : newNotes) {
        notes.push_back(note);
    }
}


Chord::Chord(Note note, ChordQuality quality, std::vector<int> intervals) : quality(quality) {
    std::vector<Note> newNotes = {note};
    for (const auto i : intervals) {
        newNotes.push_back(Note(note.midiNoteNumber+i, note.startTimeInBeats, note.durationInBeats, note.velocity));
    }
    addNotes(newNotes);
}
