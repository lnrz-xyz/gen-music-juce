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
class Chord;

class Note {
public:
    
    const int midiNoteNumber;
    const double startTimeInBeats;
    const double durationInBeats;
    const float velocity;
    
    
    Note(int midiNoteNumber, double startTimeInBeats, double durationInBeats, float velocity = 0.5f)
    : midiNoteNumber(midiNoteNumber), startTimeInBeats(startTimeInBeats), durationInBeats(durationInBeats), velocity(velocity) {}
    
};
