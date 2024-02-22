/*
  ==============================================================================

    DrumsEffectProcessor.h
    Created: 21 Feb 2024 6:39:30pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include "EffectProcessor.h"


class DrumsEffectProcessor : public EffectProcessor {
public:
    
    void process(juce::AudioBuffer<float>& buffer) override;
};
