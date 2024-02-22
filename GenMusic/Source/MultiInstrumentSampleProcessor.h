/*
  ==============================================================================

    MultiInstrumentSampleProcessor.h
    Created: 15 Feb 2024 10:05:20am
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once


#include "SampleProcessor.h"

class MultiInstrumentSampleProcessor : public SampleProcessor {
public:
    
    MultiInstrumentSampleProcessor(std::map<int, std::string> filePaths);
    juce::AudioBuffer<float> getAudioForNoteNumber(int noteNumber) override;
    
private:
    // sample buffers
    std::map<int, juce::AudioBuffer<float>> audioSampleBuffers;
};
