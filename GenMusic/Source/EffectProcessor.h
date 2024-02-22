/*
  ==============================================================================

    EffectProcessor.h
    Created: 21 Feb 2024 11:51:35am
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class EffectProcessor {
public:
    virtual ~EffectProcessor() = default;
    virtual void process(juce::AudioBuffer<float>& buffer) = 0;
};
