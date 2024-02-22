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
#include "SampleProcessor.h"

const int CHUNK_SIZE = 1024;

class DefaultSynthSound : public juce::SynthesiserSound {
public:
    bool appliesToNote (int /*midiNoteNumber*/) override { return true; }
    bool appliesToChannel (int /*midiChannel*/) override { return true; }
};


// TODO take in more than one sample and find the nearest sample for any given note to pull from
// TODO drum sample voice with no envelope 
class SampleVoice : public juce::SynthesiserVoice {
public:
    SampleVoice(std::shared_ptr<SampleProcessor> samples, int identifier, float startGain = 0.8f): identifier(identifier), sampleProcessor(samples) {
        envelope.setSampleRate(44100);
        setCurrentPlaybackSampleRate(44100);
        gain.setGainLinear(startGain);
        envelope.setParameters({0.2, 0.5, 0.8, 0.4});
    }
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        auto result = dynamic_cast<DefaultSynthSound*>(sound) != nullptr;
        return result;
    }
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override {
        midiNote = midiNoteNumber;
        audioSampleBuffer = sampleProcessor->getAudioForNoteNumber(midiNote);
        audioSampleBufferIndex = 0;
        envelope.noteOn();
    }
    
    void stopNote(float velocity, bool allowTailOff) override {
        if (envelope.isActive()) {
            envelope.noteOff(); // Start the release phase
        } else {
            clearCurrentNote(); // Immediate stop
        }
        
    }

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        if (!isVoiceActive()) {
            return;
        }
        
        fmt::print("Rendering next block for voice {} {}\n", identifier, audioSampleBufferIndex);
        
        int processSize = std::min(numSamples, audioSampleBuffer.getNumSamples() - audioSampleBufferIndex);
        if (processSize > 0) {
            juce::AudioBuffer<float> copyBuffer;
            copyBuffer.setSize(audioSampleBuffer.getNumChannels(), processSize, false, false, false);
            
            for (int channel = 0; channel < audioSampleBuffer.getNumChannels(); ++channel) {
                copyBuffer.copyFrom(channel, 0, audioSampleBuffer, channel, audioSampleBufferIndex, processSize);
            }
            
            juce::dsp::AudioBlock<float> audioBlock { copyBuffer };
            gain.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
            envelope.applyEnvelopeToBuffer(copyBuffer, 0, copyBuffer.getNumSamples());
            
            for (int channel = 0; channel < copyBuffer.getNumChannels(); ++channel) {
                outputBuffer.addFrom(channel, startSample, copyBuffer, channel, 0, processSize);
            }
            
            audioSampleBufferIndex += processSize;
        }

        if (!envelope.isActive()) {
            clearCurrentNote();
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
        gain.setGainLinear(newGain);
    }
    
private:
    // sample buffers
    juce::AudioBuffer<float> audioSampleBuffer;
    juce::AudioBuffer<float> reusableCopyBuffer;
    
    // midi
    int identifier;
    
    // processing variables
    int midiNote = 0;
    int audioSampleBufferIndex = 0;
    
    // mix, envelope, effects
    juce::dsp::Gain<float> gain;
    juce::ADSR envelope;
    std::shared_ptr<SampleProcessor> sampleProcessor;
};

