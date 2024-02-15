/*
 ==============================================================================
 
 Track.cpp
 Created: 23 Jan 2024 2:41:46pm
 Author:  Benjamin Conn
 
 ==============================================================================
 */


#include "Track.h"
#include "Song.h"
#include "Note.h"
#include <JuceHeader.h>
#include "Utilities.h"
#include <fmt/core.h>

void Track::addNote(const Note& note) {
    juce::MidiMessageSequence sequence = note.toMidiSequence(song->getBPM(), song->getSampleRate());
    midiSequence.addSequence(sequence, 0);
}

void Track::addChord(const Chord& chord) {
    juce::MidiMessageSequence sequence = chord.toMidiSequence(song->getBPM(), song->getSampleRate());
    midiSequence.addSequence(sequence, 0);
}

void Track::render(juce::AudioBuffer<float>& buffer) {
    const int totalLength = static_cast<int>(song->getSampleRate() * getTotalTime());
    juce::MidiBuffer midiBuffer;
    for (int i = 0; i < midiSequence.getNumEvents(); ++i) {
        auto* midiEvent = midiSequence.getEventPointer(i);
        int sampleNumber = static_cast<int>(midiEvent->message.getTimeStamp()); // Convert time to samples
        midiBuffer.addEvent(midiEvent->message, sampleNumber);
    }
    
    synth->renderNextBlock(buffer, midiBuffer, 0, buffer.getNumSamples());
    if (processorChain) {
        const int maximumBlockSize = 1024;
        
        for (int startSample = 0; startSample < totalLength; startSample += maximumBlockSize) {
            const int blockSize = std::min(maximumBlockSize, totalLength - startSample);
            juce::dsp::AudioBlock<float> block(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), startSample, blockSize);
            juce::dsp::ProcessContextReplacing<float> context(block);
            processorChain->process(context);
        }
    }
}

double Track::getTotalTime() {
    double time = 0.0;
    // The event time stamps will be samples. Use sampleRate from song to convert to seconds.
    // The events could be in any order so keep track of the max.
    for (int i = 0; i < midiSequence.getNumEvents(); ++i) {
        auto* midiEvent = midiSequence.getEventPointer(i);
        time = std::max(time, midiEvent->message.getTimeStamp() / song->getSampleRate() + 1.0); // extra second of silence
    }
    return time;
}
