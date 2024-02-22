/*
  ==============================================================================

    MIDIRenderer.h
    Created: 21 Feb 2024 11:51:09am
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include <vector>
#include <JuceHeader.h>
#include "Note.h"

class MIDIRenderer {
public:
    MIDIRenderer(double bpm, double sampleRate);
    
    juce::MidiMessageSequence toMidiSequence(std::vector<Note>);
    
private:
    int bpm;
    double sampleRate;
};
