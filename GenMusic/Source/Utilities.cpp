/*
  ==============================================================================

    Utilities.cpp
    Created: 25 Jan 2024 12:44:45pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#include "Utilities.h"


std::vector<unsigned char> generateRandomBytes(size_t length, const std::string& seedString) {
    std::vector<unsigned char> bytes(length);
    std::hash<std::string> hasher;
    auto hashed = hasher(seedString); // Hash the string
    std::mt19937 gen(hashed); // Seed with the hashed value
    std::uniform_int_distribution<> dis(0, 255);

    for (size_t i = 0; i < length; ++i) {
        bytes[i] = static_cast<unsigned char>(dis(gen));
    }

    return bytes;
}
