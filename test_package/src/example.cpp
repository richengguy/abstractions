#include "abstractions.h"
#include <vector>
#include <string>

int main() {
    abstractions();

    std::vector<std::string> vec;
    vec.push_back("test_package");

    abstractions_print_vector(vec);
}
