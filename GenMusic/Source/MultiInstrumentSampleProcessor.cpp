/*
  ==============================================================================

    MultiInstrumentSampleProcessor.cpp
    Created: 15 Feb 2024 10:05:20am
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "MultiInstrumentSampleProcessor.h"


MultiInstrumentSampleProcessor::MultiInstrumentSampleProcessor(std::map<int, std::string> filePaths) {
    
    // iterate through the midi note, file path map and process them into the audio buffers
    
    for (auto const& [midiNote, filePath] : filePaths) {
        juce::File file(filePath);
        juce::AudioFormatManager formatManager;
        formatManager.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        
        if (reader.get() != nullptr) {
            juce::AudioBuffer<float> audioSampleBuffer((int)reader->numChannels, (int)reader->lengthInSamples);
            reader->read(&audioSampleBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
            audioSampleBuffers[midiNote] = audioSampleBuffer;
        }
    }
    
}



// TODO return copy of audio buffer
juce::AudioBuffer<float> MultiInstrumentSampleProcessor::getAudioForNoteNumber(int noteNumber) {
    
    // check if the note exists in the map
    auto it = audioSampleBuffers.find(noteNumber);

    if (it != audioSampleBuffers.end()) {
        // copy buffer
        juce::AudioBuffer<float> copyOfBuffer;
        copyOfBuffer.makeCopyOf(it->second);
        
        return copyOfBuffer;
    }
    
    throw std::runtime_error("Note not found");
}



