/*
  ==============================================================================

    Song.cpp
    Created: 23 Jan 2024 2:41:52pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "Song.h"
#include "Track.h"  // Include Track.h if methods of Song use Track


void Song::addTrack(Track* track) {
    tracks.push_back(track);
}

double Song::getTotalTime() {
    double totalTime = 0;
    for (const auto& track : tracks) {
        double trackTime = track->getTotalTime(); // Assuming Track has a method to calculate its total time
        totalTime = std::max(totalTime, trackTime);
    }
    return 60.0;
}

void Song::renderToFile(const juce::File& outputFile) {
    juce::AudioBuffer<float> buffer;
    buffer.setSize(2, static_cast<int>(sampleRate * getTotalTime())); // Stereo buffer
    buffer.clear();

    for (auto* track : tracks) {
        track->render(buffer);
    }

    // File writing logic
    outputFile.deleteFile(); // Delete existing file, if any
    if (auto fileStream = std::make_unique<juce::FileOutputStream>(outputFile)) {
        if (!fileStream->failedToOpen()) {
            juce::WavAudioFormat wavFormat;
            if (auto writer = std::unique_ptr<juce::AudioFormatWriter>(wavFormat.createWriterFor(fileStream.get(), sampleRate, buffer.getNumChannels(), 16, {}, 0))) {
                writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
                fileStream.release(); // prevent deletion of the stream as writer will handle it
            }
        }
    }
}
