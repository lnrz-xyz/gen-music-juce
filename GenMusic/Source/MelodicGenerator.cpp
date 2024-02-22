/*
  ==============================================================================

    MelodicGenerator.cpp
    Created: 21 Feb 2024 11:50:43am
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "MelodicGenerator.h"


MelodicGenerator::MelodicGenerator(const std::vector<unsigned char> seed, const std::vector<int> roots, const std::vector<Chord> chords, bool isMajor) : seed(seed), roots(roots), chords(chords), isMajor(isMajor) {
  
}

std::vector<Note> MelodicGenerator::generate() {
    const auto melodyRhythm = generateMelodyRhythm(std::vector<unsigned char>(seed.begin(), seed.end()));
    
    const auto melodyStartingNotes = generateMelodyStartingNotes(std::vector<unsigned char>(seed.begin()+ melodyRhythm.first, seed.begin()+melodyRhythm.first + 5));
    // 4 because starting notes already used 4 bytes
    return generateMelody(std::vector<unsigned char>(seed.begin()+melodyRhythm.first + 4, seed.end()), melodyRhythm.second, melodyStartingNotes);
}
