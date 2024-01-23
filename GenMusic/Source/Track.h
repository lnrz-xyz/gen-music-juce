#pragma once
#include "Notes.h"
#include <vector>

class Song; // Forward declaration

class Track {
public:
    Track(Song* song, juce::Synthesiser* synth) : song(song), synth(synth) {}

    void addNote(const Note& note);
    void addChord(const Chord& chord);
    void render(juce::AudioBuffer<float>& buffer);
    double getTotalTime();

private:
    Song* song;
    juce::Synthesiser* synth;
    juce::MidiMessageSequence midiSequence;
};
