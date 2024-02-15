/*
 ==============================================================================
 
 Music.h
 Created: 23 Jan 2024 2:17:50pm
 Author:  Benjamin Conn
 
 ==============================================================================
 */

#pragma once
#include <vector>
#include <map>
#include <JuceHeader.h>
#include <algorithm>
#include <cctype>
#include "Utilities.h"
#include "Chord.h"
#include <fmt/core.h>

enum class NoteName {
    C, CSharp, DFlat, D, DSharp, EFlat, E, F, FSharp, GFlat, G, GSharp, AFlat, A, ASharp, BFlat, B
};

extern std::map<NoteName, int> noteToMidiMap;
extern std::map<std::string, NoteName> stringToNoteMap;

class Chord;

class Note {
public:
    
    const int midiNoteNumber;
    const double startTimeInBeats;
    const double durationInBeats;
    const float velocity;
    
    
    Note(int midiNoteNumber, double startTimeInBeats, double durationInBeats, float velocity = 0.5f)
    : midiNoteNumber(midiNoteNumber), startTimeInBeats(startTimeInBeats), durationInBeats(durationInBeats), velocity(velocity) {}
    juce::MidiMessageSequence toMidiSequence(double bpm, double sampleRate) const {
        
        double startTimeInSeconds = startTimeInBeats * (60.0 / bpm);
        double durationInSeconds = durationInBeats * (60.0 / bpm);
        
        auto startSample = static_cast<int>(startTimeInSeconds * sampleRate);
        auto endSample = startSample + static_cast<int>(durationInSeconds * sampleRate);
        
        juce::MidiMessageSequence sequence;
        sequence.addEvent(juce::MidiMessage::noteOn(1, midiNoteNumber, velocity), startSample);
        sequence.addEvent(juce::MidiMessage::noteOff(1, midiNoteNumber), endSample);
        
        return sequence;
    }
    
};
