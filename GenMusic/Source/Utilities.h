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
T selectWeightedRandom(const std::vector<std::pair<T, int>>& items, int randomNumber) {
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

    throw std::runtime_error("Error in selectWeightedRandom: Item selection failed.");
}
