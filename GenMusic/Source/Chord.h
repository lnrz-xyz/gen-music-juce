/*
  ==============================================================================

    Chord.h
    Created: 25 Jan 2024 12:54:36pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once

#include "Note.h"
#include <fmt/core.h>
#include <fmt/format.h>

class Note;

enum ChordQuality {
    Major, Minor, Diminished, Augmented, DominantSeventh, DominantThirteenth, MajorSeventh, MinorSeventh, HalfDiminishedSeventh, DiminishedSeventh, AugmentedSeventh, SusSecond, SusFourth, DominantSeventhSusFourth, DominantSeventhAltered, DominantThirteenthFlatNine, MinorMajorSeventh, MinorSixth
};

/*
    The following map contains the intervals for each chord quality. These represent the valid notes for each
    chord quality. They are also weighted so that the more "prominent" notes in a chord quality show up more.
 */
const std::map<ChordQuality, std::vector<int>> noteChoicesForQuality = {
    {Major, {0, 2, 2, 4, 4, 7, 7, 9}},
    {Minor, {0, 3, 3, 5, 5, 7, 7, 10}},
    {DominantSeventh, {0, 2, 2, 4, 4, 7, 7, 10}},
    {DominantThirteenth, {0, 2, 2, 4, 4, 7, 7, 10}},
    {MajorSeventh, {0, 2, 2, 4, 4, 7, 7, 11}},
    {MinorSeventh, {0, 3, 3, 5, 5, 7, 7, 10}},
    {MinorSixth, {0, 3, 3, 5, 5, 7, 9, 10}},
    {SusFourth, {0, 5, 5, 7, 7, 10, 10, 12}},
    {DominantSeventhSusFourth, {0, 5, 5, 7, 7, 10, 10, 12}},
    {Diminished, {0, 3, 3, 6, 6, 9, 9, 12}},
};

class Chord {
public:
    
    Chord(std::vector<Note> newNotes, ChordQuality quality) : notes(newNotes), quality(quality) {}
    Chord(Note note, ChordQuality quality, std::vector<int> intervals);
    
    void addNote(const Note& note) {
        notes.push_back(note);
    }
    
    void addNotes(std::vector<Note> notes);
    
    juce::MidiMessageSequence toMidiSequence(double bpm, double sampleRate) const;
    
    std::vector<Note> getNotes() const {
        return notes;
    }
    
    ChordQuality getQuality() const { return quality; }
    
    std::vector<int> getNoteChoices() const {
        return noteChoicesForQuality.at(quality);
    }
    
private:
    std::vector<Note> notes;
    ChordQuality quality;
};
