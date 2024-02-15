/*
  ==============================================================================

    Utilities.h
    Created: 25 Jan 2024 12:24:04pm
    Author:  Benjamin Conn

  ==============================================================================
*/

#pragma once
#include <random>
#include <vector>
#include <functional>
#include <iostream>
#include <vector>

extern std::vector<unsigned char> generateRandomBytes(size_t length, const std::string& seedString);

template <typename T>
inline T selectWeightedRandom(const std::vector<std::pair<T, int>>& items, int randomNumber) {
    int totalWeight = 0;
    for (const auto& item : items) {
        totalWeight += item.second;
    }

    // Truncate the random number if it's out of range
    randomNumber = randomNumber % totalWeight;

    for (const auto& item : items) {
        if (randomNumber < item.second)
            return item.first;
        randomNumber -= item.second;
    }

    return items.back().first;
}

inline bool basicChance(unsigned char value, double percentage) {
    // Calculate the cutoff based on the percentage
    // Maximum value for unsigned char is 255, so we multiply the percentage by 256
    // to get the range of values that should return true.
    unsigned int cutoff = static_cast<unsigned int>(256 * percentage);

    // Convert the unsigned char to an unsigned int to make the comparison straightforward
    unsigned int intValue = static_cast<unsigned int>(value);

    // If the intValue is within the cutoff, we return true.
    return intValue < cutoff;
}
