/*
  ==============================================================================

    ChordalGenerator.cpp
    Created: 21 Feb 2024 11:50:54am
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "ChordalGenerator.h"


ChordalGenerator::ChordalGenerator(const std::vector<unsigned char> seed, bool isMajor) : seed(seed), isMajor(isMajor) {
}

std::vector<Note> ChordalGenerator::generate() {
    std::vector<Chord> chords = getChords(getRoots());
    std::vector<Note> allChordNotes;
    
    
    for (const auto& chord: chords) {
        for (const auto& note: chord.getNotes()) {
            allChordNotes.push_back(note);
        }
    }
    
    return allChordNotes;
}

std::vector<Chord> ChordalGenerator::getChords(const std::vector<int> roots) {
    std::vector<Chord> result;
    
    // The first root is the key of the loop and the first chord will be in the relative diatonic chord pattern of position 0.
    // For every other chord it will be the relative diatonic pattern relative to the key
    
    const auto& root = roots[0];
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
            
            result.push_back(Chord(Note(LOW_C+currentRoot, (l*16) + i*4.0, 4.0), chordQuality, chord));
        }
    }
    
    return result;
}

std::vector<int> ChordalGenerator::getRoots() {
    std::vector<int> resultIntervals;
    
    const auto key = seed[0] % 12;
    
    for (int b = 0; b < BAR_COUNT; ++b) {
        if (b == 0) {
            resultIntervals.push_back(key);
            continue;
        }
        if (isMajor) {
            resultIntervals.push_back(key + majorScaleIntervals[seed[b] % majorScaleIntervals.size()]);
        } else {
            resultIntervals.push_back(key + minorScaleIntervals[seed[b] % minorScaleIntervals.size()]);
        }
    }
    
    return resultIntervals;
}
