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
class SampleVoice : public juce::SynthesiserVoice {
public:
    SampleVoice(std::shared_ptr<SampleProcessor> samples, int identifier, float startGain = 0.8f): identifier(identifier), sampleProcessor(samples) {
        envelope.setSampleRate(44100);
        setCurrentPlaybackSampleRate(44100);
        gain.setGainLinear(startGain);
    }
    bool canPlaySound(juce::SynthesiserSound* sound) override {
        auto result = dynamic_cast<DefaultSynthSound*>(sound) != nullptr;
        return result;
    }
    
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override {
        midiNote = midiNoteNumber;
        audioSampleBuffer = sampleProcessor->getAudioForNoteNumber(midiNote);
        audioSampleBufferIndex = 0;
        fmt::print("Starting note {} {} {} {}\n", midiNote, audioSampleBuffer.getNumSamples(), isVoiceActive(),identifier);
        // attack: seconds, decay: seconds, sustain: 0-1, release: seconds
        envelope.setParameters({0.1, 0.2, 0.5, 0.2});
        envelope.noteOn();
    }
    
    void stopNote(float velocity, bool allowTailOff) override {
        fmt::print("Stopping note {} {} {} {} {}\n", midiNote, audioSampleBufferIndex, isVoiceActive(), allowTailOff, identifier);
        if (envelope.isActive()) {
            envelope.noteOff(); // Start the release phase
        } else {
            fmt::print("Stopping note immediately {} {} {}\n", midiNote, isVoiceActive(), identifier);
            clearCurrentNote(); // Immediate stop
        }
        
    }

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        if (!isVoiceActive()) {
            fmt::print("Returning early {} {}\n", midiNote, identifier);
            return;
        }
        
        int processSize = std::min(numSamples, audioSampleBuffer.getNumSamples() - audioSampleBufferIndex);
        fmt::print("Rendering block {} {} {} {}\n", midiNote, startSample, audioSampleBufferIndex, processSize);
        if (processSize > 0) {
            juce::AudioBuffer<float> copyBuffer;
            copyBuffer.setSize(audioSampleBuffer.getNumChannels(), processSize, false, true, false);
            
            for (int channel = 0; channel < audioSampleBuffer.getNumChannels(); ++channel) {
                copyBuffer.copyFrom(channel, 0, audioSampleBuffer, channel, audioSampleBufferIndex, processSize);
            }
            
            juce::dsp::AudioBlock<float> audioBlock { copyBuffer };
            gain.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
            envelope.applyEnvelopeToBuffer(copyBuffer, 0, copyBuffer.getNumSamples());
            
            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel) {
                outputBuffer.addFrom(channel, startSample, copyBuffer, channel, 0, processSize);
            }
            
            audioSampleBufferIndex += processSize;
        }

        if (!envelope.isActive()) {
            fmt::print("Envelope is inactive {} {}\n", midiNote, identifier);
            clearCurrentNote();
        }

        if (audioSampleBufferIndex >= audioSampleBuffer.getNumSamples()) {
            fmt::print("Sample ending for note {} {}\n", midiNote, identifier);
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

