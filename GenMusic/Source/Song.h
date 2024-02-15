#pragma once
#include "Track.h"
#include <vector>
#include <JuceHeader.h>

class Song {
public:
    Song(double bpm, double sampleRate) : bpm(bpm), sampleRate(sampleRate) {}

    void addTrack(Track* track);
    double getBPM() const { return bpm; }
    double getSampleRate() const { return sampleRate; } // Assume this is set somewhere
    double getTotalTime();
    void renderToFile(const juce::File& outputFile);

private:
    std::vector<Track*> tracks;
    double bpm;
    double sampleRate; // Set this appropriately
};
