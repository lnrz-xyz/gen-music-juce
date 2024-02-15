/*
  ==============================================================================

    GrooveTrack.cpp
    Created: 14 Feb 2024 1:50:17pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "GrooveTrackGenerator.h"
#include <JuceHeader.h>
#include "Utilities.h"
#include <fmt/core.h>


GrooveTrackGenerator::GrooveTrackGenerator(int midiNoteNumber, int loops, int bars, std::vector<unsigned char> seed, std::vector<double> weighting, double grooveLength, std::vector<double> subdivisions, std::vector<int> subdivisionWeights) : seed(seed), midiNoteNumber(midiNoteNumber), loops(loops), bars(bars), playWeighting(weighting), grooveLength(grooveLength), subdivisions(subdivisions), subdivisionWeighting(subdivisionWeights) {
    // verify that the playWeighting are all chances 0-1
    for (auto& weight : playWeighting) {
        if (weight < 0 || weight > 1) {
            throw std::invalid_argument("All playWeighting must be between 0 and 1");
        }
    }
    
    // find the smallest subdivision
    std::pair<double, double> subdivisionRangeInclusive = getSubdivisionRangeInclusive();
    
    // verify that weighting length is equal to grooveLength / the smallest subdivision
    if (weighting.size() != (grooveLength / subdivisionRangeInclusive.first)) {
        throw std::invalid_argument("Weighting length must be equal to grooveLength / the smallest subdivision");
    }
    // verify that the seed length is at least the length of grooveLength / the smallest subdivision
    if (seed.size() < (grooveLength / subdivisionRangeInclusive.first)) {
        throw std::invalid_argument("Seed length must be at least the length of grooveLength / the smallest subdivision");
    }
}


std::vector<Note> GrooveTrackGenerator::generate() {
    std::vector<Note> notes;
    GrooveTrackContext ctx;
    std::pair<double, double> subdivisionRangeInclusive = getSubdivisionRangeInclusive();
    
    
    for (int loop = 0; loop < loops; ++loop ) {
        for (int bar = 0; bar < bars * 4; bar+=grooveLength) {
            for (double groovePos = 0.0; groovePos < grooveLength; ) {
                
                //  int relIndex = static_cast<int>((currentValue / doubleA) * vec.size());
                int idx = static_cast<int>((groovePos / grooveLength) * playWeighting.size());
                fmt::print("idx: {}\n", idx);
                double playChance = playWeighting.at(idx);
                unsigned char seedVal = seed.at(idx + (loops * grooveLength));
                fmt::print("seedVal: {}\n", seedVal);
                if (ctx.playCompensation > 0.0) {
                    playChance /= ctx.playCompensation; // this is a very simple implementation, we probably want to flesh out what it means to compensate for a note
                }
                
                
                std::vector<std::pair<double, int>> subdivisionToWeight;
                for (int j = 0; j < subdivisions.size(); j++) {
                    double sub = subdivisions.at(j);
                    int subdivisionWeight = subdivisionWeighting.at(j);
                    if (ctx.subdivisionCompensation[j] > 0.0) {
                        subdivisionWeight /= ctx.subdivisionCompensation[j];
                    }
                    subdivisionToWeight.push_back(std::make_pair(sub, subdivisionWeight));
                }
                
                
                double subToUse = selectWeightedRandom(subdivisionToWeight, static_cast<int>(seedVal));
                int indexOfSubToUse = 0;
                
                for (int j = 0; j < subdivisions.size(); j++) {
                    if (subdivisions.at(j) == subToUse) {
                        indexOfSubToUse = j;
                        break;
                    }
                }
                
                if (basicChance(seedVal, playChance)) {
                    double loc = (loop * bars * 4) + bar + groovePos;
                    fmt::print("loc: {}\n", loc);
                    notes.push_back(Note(midiNoteNumber, loc, subToUse));
                    ctx.playCompensation+=std::max(subToUse*2, 1.0);
                    ctx.subdivisionCompensation[indexOfSubToUse]+=std::max(subToUse*2, 1.0);
                } else {
                    ctx.playCompensation-=std::max(subToUse*2, 1.0);
                    ctx.subdivisionCompensation[indexOfSubToUse]-=std::max(subToUse*2, 1.0);
                }
                
                groovePos += subToUse;
            }
        }
    }
    
    return notes;
}
