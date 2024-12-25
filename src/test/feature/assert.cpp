#include <abstractions/errors.h>
#include <fmt/format.h>

#include "support.h"

ABSTRACTIONS_FEATURE_TEST() {
    using abstractions::errors::AbstractionsError;

    try {
        const int x = 5;
        abstractions_assert(x != 5);
        console.Print(fmt::format("'x' = {}", x));
    } catch (const AbstractionsError &exc) {
        console.Print("Hit expected assert:\n");
        exc.Print();
    }
}

ABSTRACTIONS_FEATURE_TEST_MAIN("assert", "Test out the 'assert' functions.")
