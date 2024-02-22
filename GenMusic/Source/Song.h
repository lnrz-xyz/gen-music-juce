#pragma once
#include <vector>
#include <JuceHeader.h>
#include "NoteGenerator.h"
#include "AudioProcessingBus.h"
#include "MIDIRenderer.h"
#include "EffectProcessor.h"

class Song {
public:
    Song(double bpm, double sampleRate);
    void renderToFile(const juce::File& outputFile, const juce::AudioBuffer<float>& buffer);
    void renderToMidiFile(const juce::File &outputFile, const juce::MidiMessageSequence& sequence);
    
    juce::AudioBuffer<float> generateSong(std::vector<std::pair<int, std::pair<NoteGenerator*, juce::Synthesiser*>>> noteGenerators, std::map<int, AudioProcessingBus> busses, std::map<int, EffectProcessor*> effects);
    
    juce::MidiMessageSequence generateMidi(std::vector<NoteGenerator*> noteGenerators);

private:
    double bpm;
    double sampleRate;
    MIDIRenderer midiRenderer;
};
