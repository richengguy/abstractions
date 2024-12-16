#include <abstractions/utilities.h>

#include <fmt/format.h>

int main(int nargs, const char **args)
{
    try
    {
        const int x = 5;
        abstractions_assert(x != 5);
        fmt::println("'x' = {}", x);
    }
    catch (const abstractions::errors::AbstractionsError &e)
    {
        return 0;
    }

    return 1;
}
