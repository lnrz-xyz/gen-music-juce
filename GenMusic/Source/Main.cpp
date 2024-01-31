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
const int BEATS_PER_BAR = 4.0;
const int LOOPS = 2;

// TODO use the valid notes defined in Chords.h
const std::vector<int> majorScaleIntervals = {0, 2, 4, 5, 7, 9, 11, 12};
const std::vector<int> minorScaleIntervals = {0, 2, 3, 3, 5, 5, 7, 10};

const int LOW_C = 24;

std::vector<int> generateRoots(const std::vector<unsigned char> seedForRoot, bool isMajor) {
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

const std::map<int, ChordQuality> diatonicMajorChordQualities = {
    {0, ChordQuality::Major},
    {2, ChordQuality::Minor},
    {4, ChordQuality::Minor},
    {5, ChordQuality::Major},
    {7, ChordQuality::DominantSeventh},
    {9, ChordQuality::MinorSixth},
    {11, ChordQuality::DominantSeventhSusFourth}
};
const std::map<int, std::vector<int>> diatonicMinorChordPatterns = {
    {0, {0, 15, 7}}, // root minor
    {2, {0, 15, 7}}, // minor second
    {3, {0, 16, 7}}, // major third
    {5, {0, 16, 7}}, // minor fourth
    {7, {0, 16, 10}}, // dominant7 fifth
    {10, {0, 16, 7, 11}}, // major7 seventh
    {11, {0, 16, 7}}, // major major seventh
};

const std::map<int, ChordQuality> diatonicMinorChordQualities = {
    {0, ChordQuality::Minor},
    {2, ChordQuality::Minor},
    {3, ChordQuality::Major},
    {5, ChordQuality::Minor},
    {7, ChordQuality::DominantSeventh},
    {10, ChordQuality::MajorSeventh},
    {11, ChordQuality::MajorSeventh}
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

const std::map<int, ChordQuality> diatonicMajorTransitionChordQualities = {
    {0, ChordQuality::SusFourth},
    {2, ChordQuality::Minor},
    {4, ChordQuality::DominantSeventh},
    {5, ChordQuality::MinorSixth},
    {7, ChordQuality::DominantSeventh},
    {9, ChordQuality::Minor},
    {11, ChordQuality::DominantSeventhSusFourth}
};



const std::map<int, std::vector<int>> diatonicMinorTransitionChordPatterns = {
    {0, {0, 15, 7, 11}}, // root minor
    {2, {0, 15, 7}}, // minor second
    {3, {0, 16, 7, 11}}, // major7 third
    {5, {0, 10, 15, 19}}, // minor6 fourth
    {7, {0, 16, 10}}, // dominant7 fifth
    {10, {0, 16, 10, 21}}, // dominant13 seventh
    {11, {0, 16, 7}}, // major major seventh
};

const std::map<int, ChordQuality> diatonicMinorTransitionChordQualities = {
    {0, ChordQuality::Minor},
    {2, ChordQuality::Minor},
    {3, ChordQuality::Major},
    {5, ChordQuality::MinorSixth},
    {7, ChordQuality::DominantSeventh},
    {10, ChordQuality::DominantThirteenth},
    {11, ChordQuality::Major}
};

std::vector<Chord> generateChords(const std::vector<int> roots, bool isMajor) {
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
            
            const auto& chord = isMajor ?
            (i + 1 == roots.size()) ? diatonicMajorTransitionChordPatterns.at(relativeRoot) : diatonicMajorChordPatterns.at(relativeRoot) : (i + 1 == roots.size()) ? diatonicMinorTransitionChordPatterns.at(relativeRoot) : diatonicMinorChordPatterns.at(relativeRoot);
            
            const auto chordQuality = isMajor ? (i + 1 == roots.size()) ? diatonicMajorTransitionChordQualities.at(relativeRoot) : diatonicMajorChordQualities.at(relativeRoot) : (i + 1 == roots.size()) ? diatonicMinorTransitionChordQualities.at(relativeRoot) : diatonicMinorChordQualities.at(relativeRoot);
            
            Note initial = Note(LOW_C+currentRoot, (l*16) + i*4.0, 4.0);
            
            if (l ==0) {
                fmt::println("relative root is {}, current is {}", relativeRoot, currentRoot);
                fmt::println("chord is {} ({})", fmt::join(chord, ", "), fmt::underlying(chordQuality));
                fmt::println("initial note is {}", initial.midiNoteNumber);
            }
            
            result.push_back(Chord(Note(LOW_C+currentRoot, (l*16) + i*4.0, 4.0), chordQuality, chord));
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

std::pair<int,std::vector<std::vector<double>>> generateMelodyRhythm(const std::vector<unsigned char> seedForRhythm) {
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
        const double totalSoFar = std::accumulate(currentBar.begin(), currentBar.end(), 0.0) + hangOver;
        if (totalSoFar >= BEATS_PER_BAR) {
            if (totalSoFar > BEATS_PER_BAR) {
                if (rhythm.size() + 1 == BAR_COUNT) {
                    // last note hangs over on the final bar, truncate it to be just enough to fill the 4 beats in the bar
                    fmt::println("last bar hangover, truncating: {} ({})", totalSoFar, currentBar.back());
                    currentBar.back() = 4.0 - (totalSoFar - currentBar.back());
                    fmt::println("new last note is {}", currentBar.back());
                } else {
                    // if the current bar hangs over, we store the hangover so the next bar can start with it
                    hangOver = totalSoFar - BAR_COUNT;
                    fmt::println("hangover is {}", hangOver);
                }
            } else {
                fmt:: println("no hangover");
                hangOver = 0.0;
            }
            
//            fmt::println("new bar is {}", fmt::join(currentBar, ", "));
            rhythm.push_back(currentBar);
            currentBar.clear();
            
        }
        
        
        if (rhythm.size() == BAR_COUNT) {
            break;
        }
    }
    
    return {position, rhythm};
}

std::vector<int> generateMelodyStartingNotes(const std::vector<unsigned char> seedForMelodyStartNotes, const std::vector<int> roots, const std::vector<Chord> chords, bool isMajor) {
    
    if (chords.size() != BAR_COUNT * LOOPS) {
        throw std::runtime_error("chords must be BAR_COUNT * LOOPS in length");
    }
    
    std::vector<int> startingNotes;
    
    for (int i = 0; i < BAR_COUNT; i++) {
        const auto noteChoices = chords[i].getNoteChoices();
        const auto& nextByte = seedForMelodyStartNotes[i];
        startingNotes.push_back(LOW_C + roots[i] + noteChoices[nextByte % noteChoices.size()]);
    }
    
    return startingNotes;
}

// Find the nearest note above and below the note in the note choices.
// The note will be in any octave, so we need to find the nearest note in any octave.
// (note choices are ints representing intervals in the scale, so are unoctaved 0-12 relative to root)

std::pair<int, int> nearestNotes(int note, int root, const std::vector<int>& noteChoices) {
    int nearestBelow = -1;
    int nearestAbove = 1000; // Set to a high number beyond the MIDI note range
    
    // sortedCopy is a copy of noteChoices sorted in ascending order
    std::vector<int> sortedCopy = noteChoices;
    std::sort(sortedCopy.begin(), sortedCopy.end());

    for (int interval : sortedCopy) {
        int midiNote = root + interval;

        // Loop through octaves to check for nearest notes
        for (int octave = -2; octave <= 2; ++octave) {
            int currentNote = midiNote + octave * 12;
            if (currentNote < note && currentNote > nearestBelow) {
                nearestBelow = currentNote;
            }
            if (currentNote > note && currentNote < nearestAbove) {
                nearestAbove = currentNote;
            }
        }
    }

    // Edge case: if no notes are found above or below
    if (nearestBelow == -1) nearestBelow = note; // Default to the note itself
    if (nearestAbove == 1000) nearestAbove = note; // Default to the note itself

    fmt::println("nearest below is {}, nearest above is {}", nearestBelow, nearestAbove);
    return {nearestBelow, nearestAbove};
}

struct MelodyContext {
    int lastInterval = 0;
    int compensation = 0;
    bool compensationDirectionUp = true;
};

int getNextBestNote(MelodyContext& ctx, int lastNote, std::vector<int> noteOptions, unsigned char nextSeedValue, int root) {
    
    const int lastInterval = ctx.lastInterval;
    const int absLastInterval = std::abs(lastInterval);
    const bool isUpInterval = lastInterval > 0;
    const auto nearestToLast = nearestNotes(lastNote, root, noteOptions);
    
    if (ctx.compensation > 0) {
        ctx.compensation--;
        if (ctx.compensationDirectionUp) {
            return nearestToLast.first;
        } else {
            return nearestToLast.second;
        }
    } else {
        const bool isLargeInterval = absLastInterval >= 3;
        if (isLargeInterval) {
            ctx.compensationDirectionUp = !ctx.compensationDirectionUp;
            if (absLastInterval >= 10) {
                ctx.compensation = 4;
            } else if (absLastInterval >= 7) {
                ctx.compensation = 3;
            } else {
                ctx.compensation = 2;
            }
            if (isUpInterval) {
                return nearestToLast.second;
            } else {
                return nearestToLast.first;
            }
        }
        
        const int nextNote = ((root % 12) * (lastNote / 12)) + noteOptions[nextSeedValue % noteOptions.size()];
        
        if (nextNote %12 == lastNote %12) {
            if (isUpInterval) {
                return nearestToLast.second;
            } else {
                return nearestToLast.first;
            }
        }
        
        // if the note is too far away from the last note, move it up or down an octave to make it closer
        if (std::abs(nextNote - lastNote) >= 12) {
            if (nextNote > lastNote) {
                return nearestToLast.second;
            } else {
                return nearestToLast.first;
            }
        }
        
        return nextNote;
    }
}

std::vector<Note> generateMelody(const std::vector<unsigned char> seedForMelody, const std::vector<Chord> chords, std::vector<std::vector<double>> melodyRhythm, std::vector<int> startingNotes, std::vector<int> roots) {
    
    std::vector<Note> result;
    
    MelodyContext melodyContext;
    
    std::vector<int> melodyNoteMidiValues;
    
    int seedLocation = 0;
    
    for (int i = 0; i<melodyRhythm.size(); i++) {
        const auto rhythmBar = melodyRhythm.at(i);
        const auto rootForBar = roots.at(i);
        const auto startingNote = startingNotes.at(i);
        const auto chordForBar = chords.at(i);
        const auto chordForNextBar = chords.at((i+1) % BAR_COUNT);
        const auto optionsForBar = chordForBar.getNoteChoices();
        const auto optionsForNextBar = chordForNextBar.getNoteChoices();
        
        for (int j = 0; j<rhythmBar.size(); j++) {
            // a note is a transition note if this note is going to hang into the next bar (the total value of what we have so far is greater than 4.0)
            
            const bool isTransitionNote = j == rhythmBar.size() - 1;
            
            const bool crossingBarLine = std::accumulate(rhythmBar.begin(), rhythmBar.begin() + j + 1, 0.0) > 4.0;
            
            if (isTransitionNote && j > 0) {
                const auto nextStartingNote = startingNotes.at((i+1) % BAR_COUNT);
                const int lastNote = melodyNoteMidiValues.size() > 0 ? melodyNoteMidiValues.back() : startingNote;
                
                const auto nextOptions = crossingBarLine ? optionsForNextBar : optionsForBar;
                const auto currentRoot = crossingBarLine ? roots.at((i+1) % BAR_COUNT) : rootForBar;
                const auto nearestNotesToNextStarting = nearestNotes(nextStartingNote, currentRoot, nextOptions);
                
                if (lastNote >= nextStartingNote) {
                    if (lastNote == nearestNotesToNextStarting.first) {
                        melodyNoteMidiValues.push_back(nearestNotesToNextStarting.second);
                    } else {
                        melodyNoteMidiValues.push_back(nearestNotesToNextStarting.first);
                    }
                } else {
                    if (lastNote == nearestNotesToNextStarting.second) {
                        melodyNoteMidiValues.push_back(nearestNotesToNextStarting.first);
                    } else {
                        melodyNoteMidiValues.push_back(nearestNotesToNextStarting.second);
                    }
                }
                
            } else {
                if (j == 0) {
                    melodyNoteMidiValues.push_back(startingNote);
                } else {
                    melodyNoteMidiValues.push_back(getNextBestNote(melodyContext, melodyNoteMidiValues.back(), optionsForBar, seedForMelody.at(seedLocation++), rootForBar));
                }
            }
            
            if (j > 0) {
                melodyContext.lastInterval = melodyNoteMidiValues.at(j) - melodyNoteMidiValues.back();
            }
            
            fmt::println("length: {}", melodyNoteMidiValues.size());
        }
    }
    
    double location = 0.0;
    for (int l = 0; l < LOOPS; l++) {
        int melodyIndex = 0;
        for (int i = 0; i < melodyRhythm.size(); i++) {
            for (int j = 0; j < melodyRhythm[i].size(); j++) {
                // melodyNoteMidiValues is 1:1 with the flattened melodyRhythm
                const auto note = melodyNoteMidiValues.at(melodyIndex++);
                const auto duration = melodyRhythm.at(i).at(j);
                
                if (l == 0) {
                    fmt::print("note: {}, duration: {}\n", note, duration);
                }
                
                result.push_back(Note{note, location, duration});
                location += duration;
            }
            
        }
    }
    
    return result;
    
}


// TODO bugs: sample size can overflow, some big jumps in melodes, rhythms don't always wrap around correctly
int main(int argc, char *argv[]) {
    
    std::vector<unsigned char> seed;
    if (argc > 1) {
        seed = generateRandomBytes(100, argv[1]);
    } else {
        seed = generateRandomBytes(100, "asolwijojnlksjankjn kcakjnwios");
    }
    
    int index = 0;
    
    const double bpm = 60 + 4 * (seed[index++] % 16);
    
    // Create a Synthesiser for the custom sample
    juce::Synthesiser melodySynth;
    melodySynth.setCurrentPlaybackSampleRate(SAMPLE_RATE); // Set to the desired sample rate, e.g., 44100 Hz
    melodySynth.setNoteStealingEnabled(true);
    for (int i = 0; i < 4; ++i) {
        // 4 potential notes played at once
        // TODO the note parameter doesn't really work right
        melodySynth.addVoice(new SampleVoice("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/VIOLINGRAN-C4.mp3", 36, i, 1.0f));
    }
    melodySynth.addSound(new DefaultSynthSound());
    
    juce::Synthesiser chordsSynth;
    chordsSynth.setCurrentPlaybackSampleRate(SAMPLE_RATE); // Set to the desired sample rate, e.g., 44100 Hz
    chordsSynth.setNoteStealingEnabled(true);
    for (int i = 0; i < 5; ++i) {
        // 4 potential notes played at once
        chordsSynth.addVoice(new SampleVoice("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/JPIANO-C2.mp3", 36, i, 1.0f));
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
    
    fmt::println("index for seed is {}", melodyRhythm.first);
    fmt::println("rhythm is {}", fmt::join(melodyRhythm.second, ", "));
    
    const auto melodyStartingNotes = generateMelodyStartingNotes(getSlice(seed, melodyRhythm.first, melodyRhythm.first + 4), roots, chords, isMajor);
    
    fmt::println("melody starting notes: {}", fmt::join(melodyStartingNotes,", "));
    
    // 4 because starting notes already used 4 bytes
    const auto melody = generateMelody(getSlice(seed, melodyRhythm.first + 4, seed.size()), chords, melodyRhythm.second, melodyStartingNotes, roots);
    
    for (auto& note : melody) {
        melodyTrack->addNote(note);
    }
    
    for (auto& chord : chords) {
        chordTrack->addChord(chord);
    }
    
    
    song->addTrack(melodyTrack.get());
    song->addTrack(chordTrack.get());
    
    juce::File outputFile("/Users/benjaminconn/workspace/lnrz/gen-music-script/sounds/output-v2.wav");
    
    song->renderToFile(outputFile);
    
    return 0;
}

