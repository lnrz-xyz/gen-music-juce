#pragma once
#include "Note.h"
#include <vector>
#include "WidthProcessor.h"

class Song; // Forward declaration

class Track {
public:
    // TODO less pointers, more moves of uniques
    Track(Song* song, juce::Synthesiser* synth) : song(song), synth(synth) {}
    Track(Song* song, juce::Synthesiser* synth, std::shared_ptr<juce::dsp::ProcessorChain<juce::dsp::Gain<float>, WidthProcessor, juce::dsp::Chorus<float>, juce::dsp::Reverb>> chain) : song(song), synth(synth), processorChain(std::move(chain)) {}

    void addNote(const Note& note);
    void addChord(const Chord& chord);
    void render(juce::AudioBuffer<float>& buffer);
    double getTotalTime();

private:
    Song* song;
    juce::Synthesiser* synth;
    juce::MidiMessageSequence midiSequence;
    std::shared_ptr<juce::dsp::ProcessorChain<juce::dsp::Gain<float>, WidthProcessor, juce::dsp::Chorus<float>, juce::dsp::Reverb>> processorChain;
};
