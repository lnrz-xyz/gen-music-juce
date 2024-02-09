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
    return totalTime + 1.0;
}

void Song::renderToFile(const juce::File& outputFile) {
    const int totalLength = static_cast<int>(sampleRate * getTotalTime()); // Total samples per channel
    juce::AudioBuffer<float> buffer;
    buffer.setSize(2, totalLength); // Stereo buffer
    buffer.clear(); // Fill with silence
    
    for (auto* track : tracks) {
        track->render(buffer);
    }
    
    const int maximumBlockSize = 1024;
    if (processorChain) {
        for (int startSample = 0; startSample < totalLength; startSample += maximumBlockSize) {
            const int blockSize = std::min(maximumBlockSize, totalLength - startSample);
            juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), startSample, blockSize);
            juce::dsp::ProcessContextReplacing<float> context(block);
            processorChain->process(context);
        }
    }
    
    // File writing logic
    outputFile.deleteFile(); // Delete existing file, if any
    if (auto fileStream = std::make_unique<juce::FileOutputStream>(outputFile)) {
        if (!fileStream->failedToOpen()) {
            juce::WavAudioFormat wavFormat;
            
            if (auto writer = std::unique_ptr<juce::AudioFormatWriter>(wavFormat.createWriterFor(fileStream.get(), sampleRate, buffer.getNumChannels(), 32, {}, 0))) {
                writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
                fileStream.release(); // prevent deletion of the stream as writer will handle it
            }
        }
    }
}
