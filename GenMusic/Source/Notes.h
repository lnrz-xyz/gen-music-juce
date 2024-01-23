/*
  ==============================================================================

    Music.h
    Created: 23 Jan 2024 2:17:50pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include <vector>
#include <JuceHeader.h>

class Note {
public:
    Note(int midiNoteNumber, double startTimeInBeats, double durationInBeats, float velocity = 0.8f)
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

private:
    int midiNoteNumber;
    double startTimeInBeats;
    double durationInBeats;
    float velocity;
};

class Chord {
public:
    void addNote(const Note& note) {
        notes.push_back(note);
    }

    juce::MidiMessageSequence toMidiSequence(double bpm, double sampleRate) const {
        juce::MidiMessageSequence sequence;
        for (const auto& note : notes) {
            auto noteSequence = note.toMidiSequence(bpm,sampleRate);
            sequence.addSequence(noteSequence, 0, 0, noteSequence.getEndTime());
        }
        return sequence;
    }

private:
    std::vector<Note> notes;
};
