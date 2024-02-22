/*
  ==============================================================================

    MelodicComponentsEffectProcessor.cpp
    Created: 21 Feb 2024 12:52:08pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "MelodicComponentsEffectProcessor.h"
#include <fmt/core.h>


MelodicComponentEffectProcessor::MelodicComponentEffectProcessor(double sampleRate) : processor() {
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 1024;
    spec.numChannels = 2;
    
    processor.prepare(spec);
    
    // configure the individual processors
    processor.get<0>().setGainLinear(1.0f);
    processor.get<1>().setWidth(1.3f);
    
    auto& chorus = processor.get<2>();
    
    chorus.setRate(0.9f);
    chorus.setDepth(0.2f);
    chorus.setCentreDelay(7.0f);
    chorus.setFeedback(0.0f);
    chorus.setMix(0.3f);
    
    juce::dsp::Reverb::Parameters reverbParams;
    reverbParams.roomSize = 0.7f; // Simulates a medium room
    reverbParams.damping = 0.5f;  // Balanced high-frequency decay
    reverbParams.wetLevel = 0.3f; // Noticeable reverb effect without drowning the signal
    reverbParams.dryLevel = 0.7f; // Keeps the original signal at full level
    reverbParams.width = 1.0f;    // Full stereo width for a spacious effect

    processor.get<3>().setParameters(reverbParams);
}

void MelodicComponentEffectProcessor::process(juce::AudioBuffer<float>& buffer) {
    const int maximumBlockSize = 1024;
    int numSamples = buffer.getNumSamples();
    for (int startSample = 0; startSample < numSamples; startSample += maximumBlockSize) {
        const int blockSize = std::min(maximumBlockSize, numSamples - startSample);
        juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), startSample, blockSize);
        juce::dsp::ProcessContextReplacing<float> context(block);
        processor.process(context);
    }
    
    fmt::print("Processed MelodicComponentEffectProcessor\n");
}
