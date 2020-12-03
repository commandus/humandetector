#include <inttypes.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "wavemap.h"

int main(int argc, char *argv[]) {
    std::string fn;
    if (argc < 2) {
        fn = "out.wav";
    } else {
        fn = argv[1];
    }
    WaveHeader hdr;
    WaveMap wavemap;
    wavemap.loadFiles("wav/", ".wav");
    // std::cerr << wavemap.toString();

    wavemap.say(35.1, 1);
    wavemap.say(32.2, 1);

    char buffer[44100];

    // calc size
    size_t sz = 0;
    size_t keysofs = 0;
    for (int i = 0; i <= 90; i++) {
        sz += wavemap.get(keysofs, &buffer, sizeof(buffer));
    }

    // again
    keysofs = 0;
    wavemap.say(35.1, 1);
    wavemap.say(32.2, 1);

    std::ofstream f(fn, std::ios_base::binary | std::ios_base::out);
    wavemap.header.dataBytes = sz;
    wavemap.header.wavSize = sz + sizeof(WaveHeader) - 8;
    f.write((const char *) &wavemap.header, sizeof(WaveHeader));

    for (int i = 0; i <= 90; i++) {
        size_t sz = wavemap.get(keysofs, &buffer, sizeof(buffer));
        f.write((const char *) &buffer, sz);
    }
    f.close();
}
