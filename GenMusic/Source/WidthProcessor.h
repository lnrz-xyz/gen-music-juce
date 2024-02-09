/*
  ==============================================================================

    WidthProcessor.h
    Created: 9 Feb 2024 12:44:35pm
    Author:  Benjamin Conn

  ==============================================================================
*/
#pragma once

#include <JuceHeader.h>

class WidthProcessor
{
public:
    WidthProcessor() = default;

    void prepare(const juce::dsp::ProcessSpec& spec)
    {
        // Prepare any resources or state here if needed
    }

    void process(const juce::dsp::ProcessContextReplacing<float>& context)
    {
        auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        auto numSamples = inputBlock.getNumSamples();
        auto numChannels = inputBlock.getNumChannels();

        jassert(numChannels >= 2); // Ensure we have at least a stereo signal

        for (size_t sample = 0; sample < numSamples; ++sample)
        {
            float left = inputBlock.getChannelPointer(0)[sample];
            float right = inputBlock.getChannelPointer(1)[sample];

            // Calculate mid and side signals
            float mid = (left + right) * 0.5f;
            float side = (left - right) * 0.5f;

            // Apply the matrix transformation
            outputBlock.getChannelPointer(0)[sample] = mid * (1 + widthControl) - side * widthControl; // Left
            outputBlock.getChannelPointer(1)[sample] = mid * (1 + widthControl) + side * widthControl; // Right
        }
    }

    void reset()
    {
        // Reset state if needed
    }

    void setWidth(float width)
    {
        widthControl = width;
    }

private:
    float widthControl = 1.0f; // width is [0, 2] where 1 is neutral
};
