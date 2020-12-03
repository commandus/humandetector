#include <inttypes.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "number2words.h"

int main(int argc, char *argv[]) {
    std::vector<uint64_t> numbers;
    for (int i = 1; i < argc; i++) {
        uint64_t value;
        std::istringstream iss(argv[i]);
        iss >> value;
        numbers.push_back(value);
    }
    if (!numbers.size())
         numbers.push_back(18446744073709551615U);
    for (int i = 0; i < numbers.size(); i++) {
        std::cout << number2stringRU(numbers[i]) << std::endl;
        /*
        std::vector<int32_t> codes;
        number2codeRU(codes, numbers[i]);
        for (std::vector<int32_t>::const_iterator it(codes.begin()); it < codes.end(); ++it) {
            std::cerr << *it << std::endl;
        }
        std::cerr << std::endl;
        */
    }
}
