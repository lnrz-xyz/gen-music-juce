/*
  ==============================================================================

    RepitchingSingleInstrumentSampleProcessor.h
    Created: 15 Feb 2024 10:11:21am
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once


#include <JuceHeader.h>
#include <rubberband/RubberBandStretcher.h>
#include <fmt/core.h>
#include "Note.h"
#include "SampleProcessor.h"

class RepitchingSingleInstrumentSampleProcessor : public SampleProcessor {
public:
    
    RepitchingSingleInstrumentSampleProcessor(std::string filePath, int rootMidiNote);
    RepitchingSingleInstrumentSampleProcessor(std::string filePath, int rootMidiNote, std::vector<Note> notes);
    
    juce::AudioBuffer<float> getAudioForNoteNumber(int noteNumber) override;
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

