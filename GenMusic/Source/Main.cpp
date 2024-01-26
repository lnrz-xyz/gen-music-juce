#include <JuceHeader.h>
#include <iostream>
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include "Note.h"
#include "Track.h"
#include "Song.h"
#include "Voices.h"
#include "Utilities.h"
#include <fmt/core.h>
#include <fmt/ranges.h>


const std::vector<int> majorScaleIntervals = {0, 2, 4, 5, 7, 9, 11, 12};
const std::vector<int> minorScaleIntervals = {0, 2, 3, 3, 5, 5, 7, 10};

const int LOW_C = 36;

std::vector<int> generateRoots(std::vector<unsigned char> seedForRoot, bool isMajor) {
    std::vector<int> resultIntervals;
    
    for (const auto& b : seedForRoot) {
        if (isMajor) {
            resultIntervals.push_back(majorScaleIntervals[static_cast<int>(b) % majorScaleIntervals.size()]);
        } else {
            resultIntervals.push_back(minorScaleIntervals[static_cast<int>(b) % minorScaleIntervals.size()]);
        }
    }
    
    return resultIntervals;
}


int main(int argc, char *argv[]) {
    
    std::vector<unsigned char> seed;
    if (argc > 1) {
        seed = generateRandomBytes(64, argv[1]);
    } else {
        seed = generateRandomBytes(64, "testing2");
    }
    
    int index = 0;
    
//    const double bpm = 30 + 4 * (seed[index++] % 16);
    const double bpm = 60;
    const double sampleRate = 44100.0;
    const int bars = 4;
    const int loops = 3;
    
    // Create a Synthesiser for the custom sample
    juce::Synthesiser melodySynth;
    melodySynth.setCurrentPlaybackSampleRate(sampleRate); // Set to the desired sample rate, e.g., 44100 Hz
    melodySynth.setNoteStealingEnabled(true);
    for (int i = 0; i < 5; ++i) {
        // 4 potential notes played at once
        melodySynth.addVoice(new SampleVoice("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/JVIOLIN-C2.mp3", 48, i));
    }
    melodySynth.addSound(new DefaultSynthSound());
    
    juce::Synthesiser chordsSynth;
    chordsSynth.setCurrentPlaybackSampleRate(sampleRate); // Set to the desired sample rate, e.g., 44100 Hz
    chordsSynth.setNoteStealingEnabled(true);
    for (int i = 0; i < 5; ++i) {
        // 4 potential notes played at once
        chordsSynth.addVoice(new SampleVoice("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/JCELESTE-C2.mp3", 48, i));
    }
    chordsSynth.addSound(new DefaultSynthSound());
    
    
    auto song = std::make_unique<Song>(Song(bpm, sampleRate));
    
    const auto melody = std::make_unique<Track>(Track(song.get(), &melodySynth));
    const auto chords = std::make_unique<Track>(Track(song.get(), &chordsSynth));
    const auto isMajor = static_cast<int>(seed[index++]) % 2 == 0;
    const auto rootStart = index++;
    const auto rootEnd = index+=3;
    const auto roots = generateRoots(getSlice(seed, rootStart, rootEnd), isMajor);
    
    fmt::println("roots are {}", fmt::join(roots, ", "));
    
    // first number is note pitch, second is start in beats, last is duration of note
    melody->addNote(Note(48, 0.0, 4.0));
    melody->addNote(Note(52, 2.0, 5.0));
    melody->addNote(Note(55, 7.0, 2.0));
    melody->addNote(Note(60, 9.0, 4.0));
    
    
    for (int i = 0; i < roots.size(); i++) {
        chords->addChord(Note(LOW_C+roots[i], i*4.0, 4.0).toChord({isMajor ? 4 : 3, 7}));
    }
    
    song->addTrack(melody.get());
    song->addTrack(chords.get());
    
    juce::File outputFile("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/output.wav");
    
    song->renderToFile(outputFile);
    
    return 0;
}

