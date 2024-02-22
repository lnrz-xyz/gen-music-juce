/*
  ==============================================================================

    MelodicComponentsEffectProcessor.h
    Created: 21 Feb 2024 12:52:08pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include "EffectProcessor.h"
#include "WidthProcessor.h"


class MelodicComponentEffectProcessor : public EffectProcessor {
public:
    MelodicComponentEffectProcessor(double sampleRate);
    
    void process(juce::AudioBuffer<float>& buffer) override;
private:
    juce::dsp::ProcessorChain<juce::dsp::Gain<float>, WidthProcessor, juce::dsp::Chorus<float>, juce::dsp::Reverb> processor;
};
