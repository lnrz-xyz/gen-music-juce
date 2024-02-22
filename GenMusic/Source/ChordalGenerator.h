/*
  ==============================================================================

    ChordalGenerator.h
    Created: 21 Feb 2024 11:50:54am
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include "NoteGenerator.h"
#include "Note.h"
#include <fmt/core.h>

class ChordalGenerator : public NoteGenerator {
public:
    ChordalGenerator(const std::vector<unsigned char> seed, bool isMajor);
    
    std::vector<Note> generate();
    
    std::vector<Chord> getChords(const std::vector<int> roots);
    std::vector<int> getRoots();
    
private:
    const std::vector<unsigned char> seed;
    bool isMajor;


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
        {5, {0, 15, 7}}, // minor fourth
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

};
