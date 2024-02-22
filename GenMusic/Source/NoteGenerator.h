/*
  ==============================================================================

    NoteGenerator.h
    Created: 21 Feb 2024 2:58:05pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include "Note.h"


class NoteGenerator {
public:
    static const int BAR_COUNT = 4;
    static const int LOOPS = 2;
    
    virtual ~NoteGenerator() = default;
    virtual std::vector<Note> generate() = 0;
    
protected:
    
    static const int BEATS_PER_BAR = 4.0;
    static const int LOW_C = 24;
    
    const std::vector<int> majorScaleIntervals = {0, 2, 4, 5, 7, 9, 11, 12};
    const std::vector<int> minorScaleIntervals = {0, 2, 3, 3, 5, 5, 7, 10};

    
    std::pair<int, int> nearestNotes(int note, int root, const std::vector<int>& noteChoices) {
        int nearestBelow = -1;
        int nearestAbove = 1000; // Set to a high number beyond the MIDI note range
        
        // sortedCopy is a copy of noteChoices sorted in ascending order
        std::vector<int> sortedCopy = noteChoices;
        std::sort(sortedCopy.begin(), sortedCopy.end());

        for (int interval : sortedCopy) {
            int midiNote = root + interval;

            // Loop through octaves to check for nearest notes
            for (int octave = -2; octave <= 2; ++octave) {
                int currentNote = midiNote + octave * 12;
                if (currentNote < note && currentNote > nearestBelow) {
                    nearestBelow = currentNote;
                }
                if (currentNote > note && currentNote < nearestAbove) {
                    nearestAbove = currentNote;
                }
            }
        }

        // Edge case: if no notes are found above or below
        if (nearestBelow == -1) nearestBelow = note; // Default to the note itself
        if (nearestAbove == 1000) nearestAbove = note; // Default to the note itself
        return {nearestBelow, nearestAbove};
    }
};
