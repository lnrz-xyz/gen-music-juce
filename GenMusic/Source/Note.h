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
    Note(int midiNoteNumber, double startTimeInBeats, double durationInBeats, float velocity = 0.8f)
    : midiNoteNumber(midiNoteNumber), startTimeInBeats(startTimeInBeats), durationInBeats(durationInBeats), velocity(velocity) {}
    Note(const std::string& noteName, double startTimeInBeats, double durationInBeats, float velocity = 0.8f)
    : startTimeInBeats(startTimeInBeats), durationInBeats(durationInBeats), velocity(velocity) {
        midiNoteNumber = noteNameToMidi(noteName);
    }
    
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
    
    Chord toChord(std::vector<int> intervals);
    
    int getNoteNumber() const {
        return midiNoteNumber;
    }
    
private:
    int midiNoteNumber;
    double startTimeInBeats;
    double durationInBeats;
    float velocity;
    
    
    static int noteNameToMidi(const std::string& noteString) {
        
        std::string lowerCaseNoteString = noteString;
        // Convert the string to lowercase
        std::transform(lowerCaseNoteString.begin(), lowerCaseNoteString.end(), lowerCaseNoteString.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        
        // Split the string into note name and octave
        std::string noteName;
        int octave = 0;
        for (char c : lowerCaseNoteString) {
            if (isdigit(c)) {
                octave = octave * 10 + (c - '0'); // Convert character to digit and add to octave
            } else {
                noteName += c; // Append non-digit characters to noteName
            }
        }
        
        // Find the corresponding NoteName enum
        NoteName noteEnumValue = stringToNoteMap[noteName];
        
        // Calculate and return the MIDI note number
        int baseMidiValue = noteToMidiMap[noteEnumValue];
        return baseMidiValue + octave * 12;
    }
    
};
