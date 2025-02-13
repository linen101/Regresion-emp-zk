#include <iostream>
#include <chrono>

int main() {
    using Clock = std::chrono::high_resolution_clock;
    using Duration = Clock::duration;

    std::cout << "Duration is represented in ticks of " 
              << static_cast<double>(Duration::period::num) / Duration::period::den 
              << " seconds.\n";

    return 0;
}
