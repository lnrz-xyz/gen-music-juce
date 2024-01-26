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
#include <numeric>


const double SAMPLE_RATE = 44100.0;
const int BAR_COUNT = 4;
const int LOOPS = 3;

// TODO use the valid notes defined in Chords.h
const std::vector<int> majorScaleIntervals = {0, 2, 4, 5, 7, 9, 11, 12};
const std::vector<int> minorScaleIntervals = {0, 2, 3, 3, 5, 5, 7, 10};

const int LOW_C = 24;

std::vector<int> generateRoots(const std::vector<unsigned char>& seedForRoot, bool isMajor) {
    std::vector<int> resultIntervals;
    
    const auto key = seedForRoot[0] % 12;
    
    fmt::println("key is {}", key);
    
    for (int b = 0; b < seedForRoot.size(); b++) {
        if (b == 0) {
            resultIntervals.push_back(key);
            continue;
        }
        if (isMajor) {
            resultIntervals.push_back(key + majorScaleIntervals[b % majorScaleIntervals.size()]);
        } else {
            resultIntervals.push_back(key + minorScaleIntervals[b % minorScaleIntervals.size()]);
        }
    }
    
    return resultIntervals;
}

const std::map<int, std::vector<int>> diatonicMajorChordPatterns = {
    {0, {0, 16, 7}}, // root major
    {2, {0, 15, 7}}, // minor second
    {4, {0, 15, 7}}, // minor third
    {5, {0, 16, 7}}, // major fourth
    {7, {0, 16, 10}}, // dominant7 fifth
    {9, {0, 15, 7, 10}}, // minor6 sixth
    {11, {0, 17, 10}}, // dominant7sus4 seventh
};
const std::map<int, std::vector<int>> diatonicMinorChordPatterns = {
    {0, {0, 15, 7}}, // root minor
    {2, {0, 15, 7}}, // minor second
    {3, {0, 16, 7}}, // major third
    {5, {0, 16, 7}}, // minor fourth
    {7, {0, 16, 10}}, // dominant7 fifth
    {10, {0, 16, 7, 11}}, // major7 sixth
    {11, {0, 16, 7}}, // major seventh
};

const std::map<int, std::vector<int>> diatonicMajorTransitionChordPatterns = {
    {0, {0, 17, 7}}, // root sus4
    {2, {0, 15, 7}}, // minor second
    {4, {0, 16, 10}}, // dominant7 third
    {5, {0, 10, 15, 19}}, // minor6 fourth
    {7, {0, 16, 10}}, // dominant7 fifth
    {9, {0, 15, 7, 10}}, // minor sixth
    {11, {0, 17, 10}}, // dominant7sus4 seventh
};

const std::map<int, std::vector<int>> diatonicMinorTransitionChordPatterns = {
    {0, {0, 15, 7, 11}}, // root minor
    {2, {0, 15, 7}}, // minor second
    {3, {0, 16, 7, 11}}, // major7 third
    {5, {0, 10, 15, 19}}, // minor6 fourth
    {7, {0, 16, 10}}, // dominant7 fifth
    {10, {0, 16, 10, 21}}, // dominant13 sixth
    {11, {0, 16, 7}}, // major seventh
};

std::vector<Chord> generateChords(const std::vector<int>& roots, bool isMajor) {
    std::vector<Chord> result;
    
    // The first root is the key of the loop and the first chord will be in the relative diatonic chord pattern of position 0.
    // For every other chord it will be the relative diatonic pattern relative to the key
    
    const auto& root = roots[0];
    
    fmt::println("root is {}", root);
    for (int l = 0; l < LOOPS; l++) {
        for (int i = 0; i < roots.size(); i++) {
            const auto& currentRoot = roots[i];
            auto relativeRoot = currentRoot - root;
            if (relativeRoot < 0) {
                relativeRoot += 12;
            }
            if (relativeRoot > 11) {
                relativeRoot -= 12;
            }
            
            fmt::println("relative root is {}, current is {}", relativeRoot, currentRoot);
            const auto& chord = isMajor ?
            (i + 1 == roots.size()) ? diatonicMajorTransitionChordPatterns.at(relativeRoot) : diatonicMajorChordPatterns.at(relativeRoot) : (i + 1 == roots.size()) ? diatonicMinorTransitionChordPatterns.at(relativeRoot) : diatonicMinorChordPatterns.at(relativeRoot);
            
            fmt::println("chord is {} at {}", fmt::join(chord, ", "), ((l*16) + i*4.0));
            
            result.push_back(Note(LOW_C+currentRoot, (l*16) + i*4.0, 4.0).toChord(chord));
        }
    }
    
    return result;
}

const std::vector<std::pair<double, int>> rhythmOptionsWeighted = {
    {0.25, 3},
    {1.0, 4},
    {1.5, 3},
    {2.0, 2},
    {3.0, 1},
    {4.0, 1},
};

std::pair<int,std::vector<std::vector<double>>> generateMelodyRhythm(const std::vector<unsigned char>& seedForRhythm) {
    int position = 0;
    
    // We will continually iterate through the seed to fill up BAR_COUNT bars of rhythms, keeping in mind that each bar is 4.0 beats long.
    // We return two values for this function, the first is the position in the seed that we stopped at, and the second is a vector of vectors of doubles representing the rhythm for each bar.
    std::vector<std::vector<double>> rhythm;
    std::vector<double> currentBar;
    
    double hangOver = 0.0;
    
    while (position < seedForRhythm.size()) {
        // Get the next byte from the seed
        const auto& nextByte = seedForRhythm[position++];
        
        // Get the next rhythm option from the weighted list
        const auto& rhythmOption = selectWeightedRandom(rhythmOptionsWeighted, nextByte);
        
        // Add the rhythm option to the current bar
        currentBar.push_back(rhythmOption);
        
        // If the current bar is exactly 4.0 beats long, then add it to the rhythm and clear the current bar
        const double totalSoFar = std::accumulate(currentBar.begin(), currentBar.end(), 0.0);
        if (totalSoFar >= BAR_COUNT - hangOver) {
            
            if (totalSoFar > BAR_COUNT - hangOver) {
                if (rhythm.size() + 1 == BAR_COUNT) {
                    // last note hangs over on the final bar, truncate it to be just enough to fill the 4 beats in the bar
                    fmt::println("last bar hangover, truncating");
                    currentBar.back() = 4.0 - (totalSoFar - currentBar.back());
                } else {
                    // if the current bar hangs over, we store the hangover so the next bar can start with it
                    hangOver = totalSoFar - BAR_COUNT;
                    fmt::println("hangover is {}", hangOver);
                }
            } else {
                fmt:: println("no hangover");
                hangOver = 0.0;
            }
            
            fmt::println("new bar is {}", fmt::join(currentBar, ", "));
            rhythm.push_back(currentBar);
            currentBar.clear();
            
        }
        
        
        if (rhythm.size() == BAR_COUNT) {
            break;
        }
    }
    
    return {position, rhythm};
}

std::vector<int> generateMelodyStartingNotes(const std::vector<unsigned char>& seedForMelodyStartNotes, const std::vector<Chord>& chords, bool isMajor) {
    
    if (chords.size() != BAR_COUNT * LOOPS) {
        throw std::runtime_error("chords must be BAR_COUNT * LOOPS in length");
    }
    
    std::vector<int> startingNotes;
    
    for (int i = 0; i < BAR_COUNT; i++) {
        const auto noteChoices = chords[i].getNoteChoices();
        const auto& nextByte = seedForMelodyStartNotes[i];
        startingNotes.push_back(noteChoices[nextByte % noteChoices.size()]);
    }
    
    return startingNotes;
}


int main(int argc, char *argv[]) {
    
    std::vector<unsigned char> seed;
    if (argc > 1) {
        seed = generateRandomBytes(64, argv[1]);
    } else {
        seed = generateRandomBytes(64, "kkklo0w");
    }
    
    int index = 0;
    
    const double bpm = 60 + 4 * (seed[index++] % 16);
    
    // Create a Synthesiser for the custom sample
    juce::Synthesiser melodySynth;
    melodySynth.setCurrentPlaybackSampleRate(SAMPLE_RATE); // Set to the desired sample rate, e.g., 44100 Hz
    melodySynth.setNoteStealingEnabled(true);
    for (int i = 0; i < 5; ++i) {
        // 4 potential notes played at once
        melodySynth.addVoice(new SampleVoice("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/JVIOLIN-C2.mp3", 36, i));
    }
    melodySynth.addSound(new DefaultSynthSound());
    
    juce::Synthesiser chordsSynth;
    chordsSynth.setCurrentPlaybackSampleRate(SAMPLE_RATE); // Set to the desired sample rate, e.g., 44100 Hz
    chordsSynth.setNoteStealingEnabled(true);
    for (int i = 0; i < 5; ++i) {
        // 4 potential notes played at once
        chordsSynth.addVoice(new SampleVoice("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/JPIANO-C2.mp3", 36, i));
    }
    chordsSynth.addSound(new DefaultSynthSound());
    
    
    auto song = std::make_unique<Song>(Song(bpm, SAMPLE_RATE));
    
    const auto melodyTrack = std::make_unique<Track>(Track(song.get(), &melodySynth));
    const auto chordTrack = std::make_unique<Track>(Track(song.get(), &chordsSynth));
    const auto isMajor = static_cast<int>(seed[index++]) % 2 == 0;
    
    fmt::println("is major {}", isMajor);
    
    const auto rootStart = index++;
    const auto rootEnd = index+=(BAR_COUNT-1);
    const auto roots = generateRoots(getSlice(seed, rootStart, rootEnd), isMajor);
    
    fmt::println("roots are {}", fmt::join(roots, ", "));
    
    const auto chords = generateChords(roots, isMajor);
    
    const auto melodyRhythm = generateMelodyRhythm(getSlice(seed, index, seed.size()));
    
    for (int i = 0; i < chords.size(); i++) {
        chordTrack->addChord(chords[i]);
    }
    
    song->addTrack(melodyTrack.get());
    song->addTrack(chordTrack.get());
    
    juce::File outputFile("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/output.wav");
    
    song->renderToFile(outputFile);
    
    return 0;
}

