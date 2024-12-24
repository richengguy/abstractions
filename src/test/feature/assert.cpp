#include <abstractions/errors.h>
#include <fmt/format.h>

#include "support.h"

ABSTRACTIONS_FEATURE_TEST() {
    try {
        const int x = 5;
        abstractions_assert(x != 5);
        console.Print(fmt::format("'x' = {}", x));
    } catch (const abstractions::errors::AbstractionsError &) {
        console.Print("Hit expected assert.");
    }
}

ABSTRACTIONS_FEATURE_TEST_MAIN("assert", "Test out the 'assert' functions.")
