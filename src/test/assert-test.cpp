#include <abstractions/utilities.h>

#include <fmt/format.h>

int main(int nargs, const char **args)
{
    const int x = 5;
    abstractions_assert(x != 5);
    fmt::println("'x' = {}", x);
    return 0;
}
