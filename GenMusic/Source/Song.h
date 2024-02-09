#pragma once
#include "Track.h"
#include <vector>
#include <JuceHeader.h>

class Song {
public:
    Song(double bpm, double sampleRate) : bpm(bpm), sampleRate(sampleRate) {}
    Song(double bpm, double sampleRate, std::unique_ptr<juce::dsp::ProcessorChain<juce::dsp::Gain<float>, WidthProcessor, juce::dsp::Chorus<float>, juce::dsp::Reverb>> chain) : bpm(bpm), sampleRate(sampleRate), processorChain(std::move(chain)) {}

    void addTrack(Track* track);
    double getBPM() const { return bpm; }
    double getSampleRate() const { return sampleRate; } // Assume this is set somewhere
    double getTotalTime();
    void renderToFile(const juce::File& outputFile);

private:
    std::vector<Track*> tracks;
    double bpm;
    double sampleRate; // Set this appropriately
    std::unique_ptr<juce::dsp::ProcessorChain<juce::dsp::Gain<float>, WidthProcessor, juce::dsp::Chorus<float>, juce::dsp::Reverb>> processorChain;
};
