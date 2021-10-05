#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"
#include <iostream>
#include <vector>
#include <cstring>

typedef struct
{
    int          frameIndex;  /* Index into sample array. */
    int          maxFrameIndex;
    char      *recordedSamples;
}
paTestData;

class microphone
{
private:
    int SAMPLE_RATE = 48000;
    int FRAMES_PER_BUFFER = 1024;
    int NUM_CHANNELS = 7;
    PaSampleFormat PA_SAMPLE_TYPE = paFloat32;
    std::vector<float> data;

    void initialize();
    void finalize();

public:
    // Constructor
    microphone();

    // Destructor
    ~microphone();

    std::vector<float> get_data();
};