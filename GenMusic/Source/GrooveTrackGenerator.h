/*
  ==============================================================================

    GrooveTrack.h
    Created: 14 Feb 2024 1:50:17pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include "Note.h"
#include <vector>
#include "Song.h"
#include <JuceHeader.h>
#include "NoteGenerator.h"

struct GrooveTrackContext {
    double playCompensation = 0;
    std::map<int, double> subdivisionCompensation = {};
};

// TODO convert to just a generator
class GrooveTrackGenerator : public NoteGenerator {
public:
    GrooveTrackGenerator(int midiNoteNumber, std::vector<unsigned char> seed, std::vector<double> weighting, double grooveLength, std::vector<double> subdivisions, std::vector<int> subdivisionWeights);
    std::vector<Note> generate();
    
    // TODO introduce methods to modify the weighting so that the context of the whole groove can be owned by the machine, for now it can be entirely isolated

private:
    
    
    std::vector<unsigned char> seed;
    
    int midiNoteNumber;
    std::vector<double> playWeighting;
    double grooveLength;
    std::vector<double> subdivisions;
    std::vector<int> subdivisionWeighting;
    
    std::pair<double, double> getSubdivisionRangeInclusive() {
        auto minmax = std::minmax_element(subdivisions.begin(), subdivisions.end());
        return std::make_pair(*minmax.first, *minmax.second);
    }

    
};
