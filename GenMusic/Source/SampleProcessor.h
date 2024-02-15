/*
  ==============================================================================

    SampleProcessor.h
    Created: 2 Feb 2024 9:00:09pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class SampleProcessor {
public:
    virtual ~SampleProcessor() = default;
    virtual juce::AudioBuffer<float>& getAudioForNoteNumber(int noteNumber) = 0;
};
