#include "microphone.hpp"


// Constructor
microphone::microphone()
{
    // Initialize
    initialize();
}

microphone::~microphone()
{
    // Finalize
    finalize();
}

static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    //paTestData *data = (paTestData*)userData;
    const char *rptr = (const char*)inputBuffer;
    //std::cout << data->frameIndex << std::endl;
    // for (int i = 0; i < 32; i++) {
    //     std::cout << rptr[i] << std::endl;
    // }
    std::memcpy(userData, inputBuffer, 1024 * sizeof(float));
    return 0;
}

std::vector<float> microphone::get_data() {
    std::vector<float> outData(1024, 0);
    //std::cout << data.size() << std::endl;
    if (data.size() > 0) {
        for(int i=0;i<outData.size();++i)
            outData[i] = data[i * 7] * 1000.0f;
    }

    return outData;
}

void microphone::initialize() {
    std::cout << "init audio" << std::endl;
    PaError             err = paNoError;
    err = Pa_Initialize();
    // int numDevices;
    // numDevices = Pa_GetDeviceCount();
    // if( numDevices < 0 )
    // {
    //     printf( "ERROR: Pa_CountDevices returned 0x%x\n", numDevices );
    //     err = numDevices;
    // }

    // for(int i=0; i< numDevices; i++ )
    // {
    //     auto deviceInfo = Pa_GetDeviceInfo( i );
    //     std::cout << deviceInfo->name << " : " << i << std::endl;
    //     if (i == 11) {
    //         std::cout << deviceInfo->defaultLowInputLatency << std::endl;
    //         std::cout << deviceInfo->defaultLowOutputLatency   << std::endl;
    //         std::cout << deviceInfo->defaultHighInputLatency   << std::endl;
    //         std::cout << deviceInfo->defaultHighOutputLatency   << std::endl;
            
    //         std::cout << deviceInfo->defaultSampleRate << std::endl;

    //     }


    // }

    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream*           stream;
    
    int                 i;
    int                 totalFrames;
    int                 numSamples;
    int                 numBytes;
    char                max, val;
    double              average;
    std::cout << "setting device " << std::endl;

    inputParameters.device = 11;
    inputParameters.channelCount = 7;
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
        std::cout << "setting latency " << std::endl;

    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    std::cout << "opening stream " << std::endl;

    data.resize(FRAMES_PER_BUFFER * NUM_CHANNELS);

    err = Pa_OpenStream(
        &stream,
        &inputParameters,
        NULL,                  /* &outputParameters, */
        SAMPLE_RATE,
        FRAMES_PER_BUFFER,
        paClipOff,      /* we won't output out of range samples so don't bother clipping them */
        recordCallback,
        &data[0] );

    if( err != paNoError ) {
        std::cout << "errr open" << std::endl;
    }
    err = Pa_StartStream( stream );
    if( err != paNoError ) {
        std::cout << "errr start" << std::endl;
    }
}

void microphone::finalize() {

}