/*
 ==============================================================================
 
 Voices.h
 Created: 25 Jan 2024 12:54:23pm
 Author:  Benjamin Conn
 
 ==============================================================================
 */

#pragma once
#include <JuceHeader.h>
#include <rubberband/RubberBandStretcher.h>
#include <fmt/core.h>

class DefaultSynthSound : public juce::SynthesiserSound {
public:
    bool appliesToNote (int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel (int /*midiChannel*/) override { return true; }
};

class SampleVoice : public juce::SynthesiserVoice {
public:
    SampleVoice(const std::string& samplePath, int midiNoteForSample, int identifier)
    : rootMidiNote(midiNoteForSample), identifier(identifier), currentPosition(0),
    isActive(false)
    {
        juce::File file(samplePath);
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        
        if (reader.get() != nullptr) {
            audioSampleBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
            reader->read(&audioSampleBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
        }
        
        stretcher = std::make_unique<RubberBand::RubberBandStretcher>(44100, 2,RubberBand::RubberBandStretcher::Option::OptionProcessRealTime + RubberBand::RubberBandStretcher::Option::OptionPitchHighConsistency + RubberBand::RubberBandStretcher::Option::OptionTransientsSmooth + RubberBand::RubberBandStretcher::Option::OptionPhaseIndependent + RubberBand::RubberBandStretcher::Option::OptionFormantPreserved + RubberBand::RubberBandStretcher::Option::OptionChannelsTogether + RubberBand::RubberBandStretcher::Option::OptionWindowShort + RubberBand::RubberBandStretcher::Option::OptionEngineFaster);
        
    }
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        auto result = dynamic_cast<DefaultSynthSound*>(sound) != nullptr;
        return result;
    }
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override {
        // Implement this method based on your specific requirements
        
        // Calculate the pitch shift ratio
        pitchRatio = std::pow(2.0, (midiNoteNumber - rootMidiNote) / 12.0);
        stretcher->reset();
        audioSampleBufferIndex = 0;
        isActive = true;
    }
    
    void stopNote(float velocity, bool allowTailOff) override {
        isActive = false;
    }
    
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        if (!isActive) {
            return;
        }
        stretcher->setPitchScale(pitchRatio);
        
        const int chunkSize = 256; // Or another suitable size for your application
        int processedSamples = 0;
        
        while (processedSamples < numSamples) {
            int currentChunkSize = std::min(chunkSize, numSamples - processedSamples);
            
            if (audioSampleBufferIndex + currentChunkSize <= audioSampleBuffer.getNumSamples()) {
                // Create a temporary buffer for the chunk
                juce::AudioSampleBuffer tempBuffer(audioSampleBuffer.getNumChannels(), currentChunkSize);
                for (int channel = 0; channel < audioSampleBuffer.getNumChannels(); ++channel) {
                    tempBuffer.copyFrom(channel, 0, audioSampleBuffer, channel, audioSampleBufferIndex, currentChunkSize);
                }
                
                // Process the chunk
                stretcher->process(tempBuffer.getArrayOfReadPointers(), currentChunkSize, false);
                audioSampleBufferIndex += currentChunkSize;
            } else {
                // Remaining buffer
                int remaining = audioSampleBuffer.getNumSamples() - audioSampleBufferIndex;
                juce::AudioSampleBuffer tempBuffer(audioSampleBuffer.getNumChannels(), remaining);
                for (int channel = 0; channel < audioSampleBuffer.getNumChannels(); ++channel) {
                    tempBuffer.copyFrom(channel, 0, audioSampleBuffer, channel, audioSampleBufferIndex, remaining);
                }
                
                stretcher->process(tempBuffer.getArrayOfReadPointers(), remaining, true); // 'true' for the final chunk
                audioSampleBufferIndex += remaining;
                processedSamples += remaining;
                break;
            }
            
            // Retrieve and mix the processed data
            while (stretcher->available() > 0) {
                int available = stretcher->available();
                juce::AudioSampleBuffer processedBuffer(audioSampleBuffer.getNumChannels(), available);
                stretcher->retrieve(processedBuffer.getArrayOfWritePointers(), available);
                
                for (int channel = 0; channel < processedBuffer.getNumChannels(); ++channel) {
                    outputBuffer.addFrom(channel, startSample + processedSamples, processedBuffer, channel, 0, available);
                }
                processedSamples += available;
            }
        }
        
        
        if (audioSampleBufferIndex >= audioSampleBuffer.getNumSamples()) {
            stopNote(0.0f, false); // Automatically stop the note if we've reached the end of the sample
        }
    }
    
    
    void pitchWheelMoved(int newValue) override {
        // Handle pitch wheel changes if needed
    }
    
    void controllerMoved(int controllerNumber, int newValue) override {
        // Handle other MIDI controllers if needed
    }
    
private:
    juce::AudioSampleBuffer audioSampleBuffer;
    int rootMidiNote;
    int identifier;
    double currentPosition;
    double pitchRatio;
    bool isActive;
    int audioSampleBufferIndex = 0;
    std::unique_ptr<RubberBand::RubberBandStretcher> stretcher;
};

