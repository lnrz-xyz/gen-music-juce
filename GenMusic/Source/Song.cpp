/*
 ==============================================================================
 
 Song.cpp
 Created: 23 Jan 2024 2:41:52pm
 Author:  Benjamin Conn
 
 ==============================================================================
 */

#include "Song.h"
#include <fmt/core.h>

Song::Song(double bpm, double sampleRate) : bpm(bpm), sampleRate(sampleRate), midiRenderer(bpm, sampleRate) {
    
}

void Song::renderToFile(const juce::File& outputFile, const juce::AudioBuffer<float>& buffer) {
    // File writing logic
    outputFile.deleteFile(); // Delete existing file, if any
    if (auto fileStream = std::make_unique<juce::FileOutputStream>(outputFile)) {
        if (!fileStream->failedToOpen()) {
            juce::WavAudioFormat wavFormat;
            
            if (auto writer = std::unique_ptr<juce::AudioFormatWriter>(wavFormat.createWriterFor(fileStream.get(), sampleRate, buffer.getNumChannels(), 32, {}, 0))) {
                writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
                fileStream.release(); // prevent deletion of the stream as writer will handle it
            }
        }
    }
}

juce::AudioBuffer<float> Song::generateSong(std::vector<std::pair<int, std::pair<NoteGenerator*, juce::Synthesiser*>>> noteGenerators, std::map<int, AudioProcessingBus> busses, std::map<int, EffectProcessor*> effects) {
    
    std::vector<std::pair<int, std::pair<juce::MidiMessageSequence*, juce::Synthesiser*>>> midiSequences;
    
    for (auto& noteGenerator : noteGenerators) {
        auto* seq = new juce::MidiMessageSequence(midiRenderer.toMidiSequence(noteGenerator.second.first->generate()));
        fmt::println("Generated sequence with {} events", seq->getNumEvents());
        midiSequences.push_back(std::make_pair(noteGenerator.first, std::make_pair(seq, noteGenerator.second.second)));
    }
    
    double earliestStartTime = 0;
    double latestEndTime = 0;
    for (auto sequence : midiSequences) {
        fmt::println("Processing sequence for bus {} ({})", sequence.first, sequence.second.first->getNumEvents());
        for (int i = 0; i < sequence.second.first->getNumEvents(); ++i) {
            auto message = sequence.second.first->getEventPointer(i)->message;
            double timeStamp = message.getTimeStamp();
            double timeInSeconds = (timeStamp / sampleRate);
            earliestStartTime = std::min(earliestStartTime, timeInSeconds);
            latestEndTime = std::max(latestEndTime, timeInSeconds);
        }
    }
    
    double totalTimeSpanInSeconds = latestEndTime - earliestStartTime;
    
    fmt::println("total time: {}", totalTimeSpanInSeconds);
    
    int totalSamples = static_cast<int>(totalTimeSpanInSeconds * sampleRate + (sampleRate * 2));
    
    
    juce::AudioBuffer<float> buffer;
    buffer.setSize(2, totalSamples);
    buffer.clear();
    
    for (auto& bus : busses) {
        int key = bus.first;
        std::vector<std::pair<juce::MidiMessageSequence*, juce::Synthesiser*>> midiSynthPairs;
        
        for (auto& sequence : midiSequences) {
            if (sequence.first != key) {
                continue;
            }
            fmt::println("Adding sequence for bus {} ({})", key,sequence.second.first->getNumEvents());
            midiSynthPairs.push_back(std::make_pair(sequence.second.first, sequence.second.second));
        }
        
        bus.second.render(midiSynthPairs, buffer, effects.at(key));
    }
    
    for (auto& sequence : midiSequences) {
        delete sequence.second.first;
    }

    
    
    return buffer;
}

void Song::renderToMidiFile(const juce::File &outputFile, const juce::MidiMessageSequence &sequence) {
    outputFile.deleteFile();
    juce::MidiFile midiFile;
    midiFile.addTrack(sequence);
    midiFile.setTicksPerQuarterNote(sampleRate / (bpm / 60));
    juce::FileOutputStream stream(outputFile);
    midiFile.writeTo(stream);
}

juce::MidiMessageSequence Song::generateMidi(std::vector<NoteGenerator *> noteGenerators) {
    std::vector<Note> allNotes;
    for (auto& generator : noteGenerators) {
        auto notes = generator->generate();
        for (auto& note : notes) {
            allNotes.push_back(note);
        }
    }
    
    return midiRenderer.toMidiSequence(allNotes);
}


