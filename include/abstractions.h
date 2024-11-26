#pragma once

#include <vector>
#include <string>


#ifdef _WIN32
  #define ABSTRACTIONS_EXPORT __declspec(dllexport)
#else
  #define ABSTRACTIONS_EXPORT
#endif

ABSTRACTIONS_EXPORT void abstractions();
ABSTRACTIONS_EXPORT void abstractions_print_vector(const std::vector<std::string> &strings);
