#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "wavemap.h"

int main(int argc, char *argv[]) {
    std::string fn;
    std::vector<double> numbers;
    if (argc < 2) {
        fn = "out.wav";
    } else {
        fn = argv[1];
        for (int i = 2; i < argc; i++) {
            double value;
            std::istringstream iss(argv[i]);
            iss >> value;
            numbers.push_back(value);
        }
    }
    if (!numbers.size()) {
        numbers.push_back(35.1);
        numbers.push_back(32.2);
    }

    WaveHeader hdr;
    WaveMap wavemap;
    wavemap.loadFiles("wav/", ".wav");
    // std::cerr << wavemap.toString();

    for (std::vector<double>::const_iterator it(numbers.begin()); it != numbers.end(); it++) {
        wavemap.say(*it, 1);
    }

    char buffer[44100];

    // calc size
    size_t sz = 0;
    size_t keysofs = 0;
    while (true) {
        sz += wavemap.get(keysofs, &buffer, sizeof(buffer));
        if (!wavemap.hasQueuedSentence())
            break;
    }

    // again
    keysofs = 0;
    for (std::vector<double>::const_iterator it(numbers.begin()); it != numbers.end(); it++) {
        wavemap.say(*it, 1);
    }

    std::ofstream f(fn, std::ios_base::binary | std::ios_base::out);
    wavemap.header.dataBytes = sz;
    wavemap.header.wavSize = sz + sizeof(WaveHeader) - 8;
    f.write((const char *) &wavemap.header, sizeof(WaveHeader));

    while (true) {
        size_t sz = wavemap.get(keysofs, &buffer, sizeof(buffer));
        f.write((const char *) &buffer, sz);
        if (!wavemap.hasQueuedSentence())
            break;
    }
    f.close();
}
