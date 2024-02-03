/*
 ==============================================================================
 
 SampleProcessor.cpp
 Created: 2 Feb 2024 9:00:09pm
 Author:  Benjamin Conn
 
 ==============================================================================
 */

#include "SampleProcessor.h"
#include <JuceHeader.h>
#include <rubberband/RubberBandStretcher.h>
#include <fmt/core.h>
#include "Note.h"


SampleProcessor::SampleProcessor(std::string filePath, int rootMidiNote) : rootMidiNote(rootMidiNote) {
    // Load the audio file
    juce::File file(filePath);
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    
    if (reader.get() != nullptr) {
        originalAudioSampleBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&originalAudioSampleBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    }
    
    // Create the stretcher
    stretcher = std::make_shared<RubberBand::RubberBandStretcher>(44100, 2, RubberBand::RubberBandStretcher::OptionProcessOffline + RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency + RubberBand::RubberBandStretcher::Option::OptionEngineFiner);
}

SampleProcessor::SampleProcessor(std::string filePath, int rootMidiNote, std::vector<Note> notes) : rootMidiNote(rootMidiNote) {
    juce::File file(filePath);
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    
    if (reader.get() != nullptr) {
        originalAudioSampleBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
        reader->read(&originalAudioSampleBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    }
    
    // Create the stretcher
    stretcher = std::make_shared<RubberBand::RubberBandStretcher>(44100, 2, RubberBand::RubberBandStretcher::OptionProcessOffline + RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency + RubberBand::RubberBandStretcher::Option::OptionEngineFiner);
    for (auto note : notes) {
        getAudioForNoteNumber(note.midiNoteNumber);
    }
}


const juce::AudioBuffer<float>& SampleProcessor::getAudioForNoteNumber(int noteNumber) {
    // check if the note exists in the map
    
    // if it does, return the buffer
    // if it doesn't, process the audio and add it to the map
    
    auto it = reprocessedAudioSampleBuffers.find(noteNumber);

    if (it != reprocessedAudioSampleBuffers.end()) {
        fmt::print("Note {} already exists\n", noteNumber);
        return reprocessedAudioSampleBuffers[noteNumber];
    }
    
    // process the audio
    
    juce::AudioBuffer<float> copy(originalAudioSampleBuffer.getNumChannels(), originalAudioSampleBuffer.getNumSamples());
    for (int i = 0; i < originalAudioSampleBuffer.getNumChannels(); ++i) {
        copy.copyFrom(i, 0, originalAudioSampleBuffer, i, 0, originalAudioSampleBuffer.getNumSamples());
    }
    
    double pitchRatio = std::pow(2.0, (noteNumber - rootMidiNote) / 12.0);
    stretcher->reset();
    stretcher->setPitchScale(pitchRatio);
    stretcher->setExpectedInputDuration(originalAudioSampleBuffer.getNumSamples());
    
    // first study the whole audio
    stretcher->study(copy.getArrayOfReadPointers(), copy.getNumSamples(), true);
    
    int samplesSent = 0;
    int processed = 0;
    std::vector<std::vector<float>> processedSamples(originalAudioSampleBuffer.getNumChannels());
    
    while (true) {
        int chunkSize = stretcher->getSamplesRequired();
        int actualSend = std::min(chunkSize, originalAudioSampleBuffer.getNumSamples() - samplesSent);
        copy.setSize(originalAudioSampleBuffer.getNumChannels(), actualSend, false, true, false);
        for (int channel = 0; channel < originalAudioSampleBuffer.getNumChannels(); ++channel) {
            copy.copyFrom(channel, 0, originalAudioSampleBuffer, channel, samplesSent, actualSend);
        }
        bool doneSending = actualSend < chunkSize || samplesSent + actualSend >= originalAudioSampleBuffer.getNumSamples();
        bool sentFinal = false;
        
        if (!sentFinal) {
            stretcher->process(copy.getArrayOfReadPointers(), actualSend, doneSending);
            samplesSent += actualSend;
            sentFinal = doneSending;
        }
        
        while (stretcher->available() > 0) {
            int available = stretcher->available();
            juce::AudioBuffer<float> processedBuffer(originalAudioSampleBuffer.getNumChannels(), available);
            stretcher->retrieve(processedBuffer.getArrayOfWritePointers(), available);
            
            for (int i = 0; i < processedBuffer.getNumChannels(); ++i) {
                for (int j = 0; j < processedBuffer.getNumSamples(); ++j) {
                    processedSamples[i].push_back(processedBuffer.getSample(i, j));
                }
            }
            processed += available;
        }
        
        if (stretcher->available() < 0) {
            break;
        }
    }
    
    juce::AudioBuffer<float> output(originalAudioSampleBuffer.getNumChannels(), processedSamples[0].size());
    for (int channel = 0; channel < originalAudioSampleBuffer.getNumChannels(); ++channel) {
        output.copyFrom(channel, 0, processedSamples[channel].data(), processed);
    }
    
    reprocessedAudioSampleBuffers[noteNumber] = output;
    
    return output;
}
