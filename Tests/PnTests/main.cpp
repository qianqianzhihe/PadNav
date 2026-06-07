#include <cstdlib>
#include <exception>
#include <iostream>

void runPadNavControllerTests();
void runMappingEngineTests();
void runProfileTests();

int main() {
    try {
        runProfileTests();
        runMappingEngineTests();
        runPadNavControllerTests();
        std::cout << "padnav_tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
