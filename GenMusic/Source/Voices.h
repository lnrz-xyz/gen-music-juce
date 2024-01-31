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

const int CHUNK_SIZE = 1024;

class DefaultSynthSound : public juce::SynthesiserSound {
public:
    bool appliesToNote (int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel (int /*midiChannel*/) override { return true; }
};


// TODO take in more than one sample and find the nearest sample for any given note to pull from
class SampleVoice : public juce::SynthesiserVoice {
public:
    SampleVoice(const std::string& samplePath, int midiNoteForSample, int identifier, double gain = 0.8f)
    : rootMidiNote(midiNoteForSample), identifier(identifier),
    isActive(false), gain(gain)
    {
        juce::File file(samplePath);
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        
        if (reader.get() != nullptr) {
            audioSampleBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
            reader->read(&audioSampleBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
        }
        
        stretcher = std::make_unique<RubberBand::RubberBandStretcher>(44100, 2,RubberBand::RubberBandStretcher::Option::OptionProcessRealTime + RubberBand::RubberBandStretcher::Option::OptionPitchHighQuality +
        + RubberBand::RubberBandStretcher::Option::OptionFormantShifted + RubberBand::RubberBandStretcher::Option::OptionChannelsApart + RubberBand::RubberBandStretcher::Option::OptionEngineFiner + RubberBand::RubberBandStretcher::Option::OptionWindowStandard + RubberBand::RubberBandStretcher::Option::OptionThreadingAuto);
    
        
        
    }
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        auto result = dynamic_cast<DefaultSynthSound*>(sound) != nullptr;
        return result;
    }
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override {
        // Calculate the pitch shift ratio
        pitchRatio = std::pow(2.0, (midiNoteNumber - rootMidiNote) / 12.0);
        stretcher->reset();
        audioSampleBufferIndex = 0;
        reusableBuffer.clear();
        // attack: seconds, decay: seconds, sustain: 0-1, release: seconds
        envelope.setParameters({0.1, 0.2, 0.5, 1.0});
        envelope.noteOn();
        isActive = true;
        fmt::print("Note: {} {}\n", midiNoteNumber, identifier);
    }
    
    void stopNote(float velocity, bool allowTailOff) override {
        fmt::print("Releasing with tail\n");
        isActive = false;
        
        if (!allowTailOff) {
            envelope.noteOff();
            clearCurrentNote();
        }
    }

    
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        if (!isActive) {
            fmt::print("Returning early\n");
            return;
        }
        stretcher->setPitchScale(pitchRatio);
        
        // TODO stretcher delay compression
        int processedSamples = 0;
        
        while (processedSamples < numSamples) {
            const int chunkSize = stretcher->getSamplesRequired();
            int currentChunkSize = std::min(chunkSize, numSamples - processedSamples);
            
            if (audioSampleBufferIndex + currentChunkSize <= audioSampleBuffer.getNumSamples()) {
                
                reusableBuffer.setSize(audioSampleBuffer.getNumChannels(), currentChunkSize);
                for (int channel = 0; channel < audioSampleBuffer.getNumChannels(); ++channel) {
                    reusableBuffer.copyFrom(channel, 0, audioSampleBuffer, channel, audioSampleBufferIndex, currentChunkSize);
                }
                
                // Process the chunk
                stretcher->process(reusableBuffer.getArrayOfReadPointers(), currentChunkSize, false);
                audioSampleBufferIndex += currentChunkSize;
            } else {
                // Remaining buffer
                int remaining = audioSampleBuffer.getNumSamples() - audioSampleBufferIndex;
                
                reusableBuffer.setSize(audioSampleBuffer.getNumChannels(), remaining);
                for (int channel = 0; channel < audioSampleBuffer.getNumChannels(); ++channel) {
                    reusableBuffer.copyFrom(channel, 0, audioSampleBuffer, channel, audioSampleBufferIndex, remaining);
                }
                
                stretcher->process(reusableBuffer.getArrayOfReadPointers(), remaining, true); // 'true' for the final chunk
                audioSampleBufferIndex += remaining;
                processedSamples += remaining;
                break;
            }
            
            // Retrieve and mix the processed data
            while (stretcher->available() > 0) {
                int available = stretcher->available();
                juce::AudioSampleBuffer processedBuffer(audioSampleBuffer.getNumChannels(), available);
                stretcher->retrieve(processedBuffer.getArrayOfWritePointers(), available);
                
                for (int i = 0; i < available; ++i) {
                    float envelopeValue = envelope.getNextSample(); // Get the current envelope value
                    for (int channel = 0; channel < processedBuffer.getNumChannels(); ++channel) {
                        float processedSample = processedBuffer.getSample(channel, i) * gain;
                        processedBuffer.setSample(channel, i, processedSample * envelopeValue);
                    }
                }
                
                
                for (int channel = 0; channel < processedBuffer.getNumChannels(); ++channel) {
                    outputBuffer.addFrom(channel, startSample + processedSamples, processedBuffer, channel, 0, available);
                }
                processedSamples += available;
            }
        }
        
        if (audioSampleBufferIndex >= audioSampleBuffer.getNumSamples()) {
            stopNote(0.0f, true); // Automatically stop the note if we've reached the end of the sample
        }
    }
    
    void pitchWheelMoved(int newValue) override {
        // Handle pitch wheel changes if needed
    }
    
    void controllerMoved(int controllerNumber, int newValue) override {
        // Handle other MIDI controllers if needed
    }
    
    void setGain(float newGain) {
        gain = newGain;
    }
    
private:
    // sample buffers
    juce::AudioSampleBuffer audioSampleBuffer;
    juce::AudioSampleBuffer reusableBuffer;
    
    
    // midi
    int rootMidiNote;
    
    int identifier;
    
    // processing variables
    double pitchRatio;
    bool isActive;
    int audioSampleBufferIndex = 0;
    std::unique_ptr<RubberBand::RubberBandStretcher> stretcher;
    
    // mix, envelope, effects
    float gain;
    juce::ADSR envelope;
};

