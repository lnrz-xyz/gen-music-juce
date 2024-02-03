/*
  ==============================================================================

    SampleProcessor.h
    Created: 2 Feb 2024 9:00:09pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once


#include <JuceHeader.h>
#include <rubberband/RubberBandStretcher.h>
#include <fmt/core.h>
#include "Note.h"

class SampleProcessor {
public:
    
    SampleProcessor(std::string filePath, int rootMidiNote);
    SampleProcessor(std::string filePath, int rootMidiNote, std::vector<Note> notes);
    ~SampleProcessor() {}
    
    const juce::AudioBuffer<float>& getOriginalAudioSampleBuffer() { return originalAudioSampleBuffer; }
    const juce::AudioBuffer<float>& getAudioForNoteNumber(int noteNumber);
    
private:
    // sample buffers
    juce::AudioBuffer<float> originalAudioSampleBuffer;
    // the map of all of midi notes to its reprocessed audio buffer
    std::map<int, juce::AudioBuffer<float>> reprocessedAudioSampleBuffers;
    
    // midi
    int rootMidiNote;
    
    // processing variables
    std::shared_ptr<RubberBand::RubberBandStretcher> stretcher;
    
};
