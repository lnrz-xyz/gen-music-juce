/*
  ==============================================================================

    MelodicGenerator.h
    Created: 21 Feb 2024 11:50:43am
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include "NoteGenerator.h"
#include "Note.h"
#include "Chord.h"
#include <fmt/core.h>

struct MelodyContext {
    int lastInterval = 0;
    int compensation = 0;
    bool compensationDirectionUp = true;
};

class MelodicGenerator : public NoteGenerator {
public:
    MelodicGenerator(const std::vector<unsigned char> seed, const std::vector<int> roots, const std::vector<Chord> chords, bool isMajor);
    
    std::vector<Note> generate();
private:
    const std::vector<unsigned char> seed;
    const std::vector<int> roots;
    const std::vector<Chord> chords;
    bool isMajor;
    
    std::vector<Note> generateMelody(const std::vector<unsigned char> seedForMelody, std::vector<std::vector<double>> melodyRhythm, std::vector<int> startingNotes) {
        
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
                
            }
        }
        
        
        double location = 0.0;
        for (int l = 0; l < LOOPS; l++) {
            int melodyIndex = 0;
            for (int i = 0; i < melodyRhythm.size(); i++) {
                for (int j = 0; j < melodyRhythm[i].size(); j++) {
                    // melodyNoteMidiValues is 1:1 with the flattened melodyRhythm
                    auto note = melodyNoteMidiValues.at(melodyIndex++);
                    const auto duration = melodyRhythm.at(i).at(j);
                    result.push_back(Note{note, location, duration});
                    location += duration;
                }
                
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
                        currentBar.back() = 4.0 - (totalSoFar - currentBar.back());
                    } else {
                        // if the current bar hangs over, we store the hangover so the next bar can start with it
                        hangOver = totalSoFar - BAR_COUNT;
                    }
                } else {
                    hangOver = 0.0;
                }
                
                rhythm.push_back(currentBar);
                currentBar.clear();
                
            }
            
            
            if (rhythm.size() == BAR_COUNT) {
                break;
            }
        }
        
        return {position, rhythm};
    }
    
    
    std::vector<int> generateMelodyStartingNotes(const std::vector<unsigned char> seedForMelodyStartNotes) {
        
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
            
            const int nextNote = (root % 12) + noteOptions[nextSeedValue % noteOptions.size()];
            
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
};
