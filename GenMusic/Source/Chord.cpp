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


ChordQuality Chord::getQuality() const{
    
    int root = notes[0].getNoteNumber() % 12;
    // root is now 0 and every other note is it's relation to the root
    
    std::vector<int> intervals;
    
    for (int i = 1; i < notes.size(); ++i) {
        int nextNote = notes[i].getNoteNumber() % 12;
        if (nextNote < root) nextNote += 12;
        intervals.push_back(notes[i].getNoteNumber() % 12 - root);
    }
    
    std::sort(intervals.begin(), intervals.end());
    
    // Assuming the intervals are sorted
    int third = -1, fifth = -1, sixth = -1, seventh = -1;

    for (int interval : intervals) {
        int normalizedInterval = interval % 12;
        if (normalizedInterval == 4) third = Major;
        else if (normalizedInterval == 3) third = Minor;
        else if (normalizedInterval == 7) fifth = Diminished;
        else if (normalizedInterval == 8) fifth = Augmented;
        else if (normalizedInterval == 9) sixth = Major;
        else if (normalizedInterval == 10) sixth = Minor;
        else if (normalizedInterval == 10) seventh = MinorSeventh;
        else if (normalizedInterval == 11) seventh = MajorSeventh;
    }

    // Logic to determine the chord quality
    if (third == Major && fifth != Augmented) return (seventh == -1) ? Major : (seventh == MajorSeventh ? MajorSeventh : (sixth == -1) ? DominantSeventh: DominantThirteenth);
    if (third == Major && fifth == Augmented) return Augmented;
    if (third == Minor && fifth == Diminished) return (sixth == -1) ? (seventh == -1) ? Diminished : HalfDiminishedSeventh : DiminishedSeventh;
    if (third == Minor) return (sixth == Minor) ? MinorSixth :  (seventh == -1) ? Minor : seventh == MajorSeventh ? MinorSeventh : MinorMajorSeventh;
    // Default case if no chord is detected
    return Major; // You can choose a default value
}


Chord::Chord(std::vector<Note> newNotes) {
    addNotes(newNotes);
}

