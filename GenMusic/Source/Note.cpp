/*
  ==============================================================================

    Notes.cpp
    Created: 25 Jan 2024 12:22:49pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "Note.h"
#include "Chord.h"
#include "Utilities.h"

std::map<NoteName, int> noteToMidiMap = {
    {NoteName::C, 0},
    {NoteName::CSharp, 1}, {NoteName::DFlat, 1},
    {NoteName::D, 2},
    {NoteName::DSharp, 3}, {NoteName::EFlat, 3},
    {NoteName::E, 4},
    {NoteName::F, 5}, {NoteName::FSharp, 6},
    {NoteName::GFlat, 6}, {NoteName::G, 7},
    {NoteName::GSharp, 8},
    {NoteName::AFlat, 8}, {NoteName::A, 9},
    {NoteName::ASharp, 10}, {NoteName::BFlat, 10},
    {NoteName::B, 11}
};

std::map<std::string, NoteName> stringToNoteMap = {
    {"c", NoteName::C},
    {"c#", NoteName::CSharp}, {"db", NoteName::DFlat},
    {"d", NoteName::D},
    {"d#", NoteName::DSharp}, {"eb", NoteName::EFlat},
    {"e", NoteName::E},
    {"eb", NoteName::EFlat}, {"f", NoteName::F},
    {"f#", NoteName::FSharp},
    {"gb", NoteName::GFlat}, {"g", NoteName::G},
    {"g#", NoteName::GSharp},
    {"ab", NoteName::AFlat}, {"a", NoteName::A},
    {"a#", NoteName::ASharp},
    {"bb", NoteName::BFlat}, {"b", NoteName::B},
};
