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

template<typename T>
std::vector<T> getSlice(const std::vector<T>& v, int start, int end) {
    // Adjust end if it's beyond the vector's length
    end = (end > v.size()) ? v.size() : end;

    // Check if start is within the vector
    if (start < 0 || start >= v.size() || start > end) {
        return std::vector<T>(); // Return an empty vector in case of invalid range
    }

    // Create a new vector with a slice of the original vector
    std::vector<T> slice(v.begin() + start, v.begin() + end);

    return slice;
}
