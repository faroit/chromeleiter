#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"
#include "Chromagram.h"
#include "ChordDetector.h"
#include <iostream>
#include <cstdlib>
#include <numeric>
#include "RtMidi.h"
#include <cstdlib>
#include <string>

using namespace std;

#define max(a,b) (a>b?a:b)

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (2048)
#define NUM_SECONDS     (5)
#define NUM_CHANNELS    (1)

/* Select sample format. */
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#endif

Chromagram* c = 0;
ChordDetector* cd = 0;
RtMidiOut *midiout = 0;

int r = 0;
int g = 0;
int b = 0;

typedef struct
{
    int          frameIndex;  /* Index into sample array. */
    int          maxFrameIndex;
    SAMPLE      *recordedSamples;
    std::vector<double> chromagram;
}
paTestData;

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/

int note_to_key(int note, int octave) {
    // needs to simplified
    int k = 12 + (12 * octave) + note;
    switch(k) {
         case 19 :
         case 29 :
         case 39 :
         case 49 :
         case 59 :
         case 69 :
         case 79 :
         case 89 :
            k += 1;
      }
      return k;
}

static int recordCallback( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paTestData *data = (paTestData*)userData;
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    SAMPLE *wptr = &data->recordedSamples[data->frameIndex * NUM_CHANNELS];
    long framesToCalc;
    long i;
    int finished;
    unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;
    double frame[FRAMES_PER_BUFFER];

    (void) outputBuffer; /* Prevent unused variable warnings. */
    (void) timeInfo;
    (void) statusFlags;
    (void) userData;


    if( framesLeft < framesPerBuffer )
    {
        framesToCalc = framesLeft;
        finished = paComplete;
    }
    else
    {
        framesToCalc = framesPerBuffer;
        finished = paContinue;
    }

    if( inputBuffer == NULL )
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = SAMPLE_SILENCE;  /* left */
            if( NUM_CHANNELS == 2 ) *wptr++ = SAMPLE_SILENCE;  /* right */
        }
    }
    else
    {
        for( i=0; i<framesToCalc; i++ )
        {
            *wptr++ = *rptr++;  /* left */
            frame[i] = (double)*rptr;
            data->frameIndex = frame[i];
            if( NUM_CHANNELS == 2 ) *wptr++ = *rptr++;  /* right */
        }
    }
    c->processAudioFrame(frame);
    if (c->isReady()) {
        data->chromagram = c->getChromagram();
        std::vector<unsigned char> message;

        for (int i = 0;i < 12;i++)
        {
            cd->detectChord(data->chromagram);
            for (int j = 0; j < 7; j++) {
                double sum = std::  accumulate(data->chromagram.begin(), data->chromagram.end(), 0.0);
                double mean = sum / data->chromagram.size();
                int k = note_to_key(i, j);
                int r = note_to_key(cd->rootNote, j);
                int m = cd->quality == 0 ? 2 : 0;

                r = m * 8 * max((int)data->chromagram[i] - (int)mean, 0);
                b = 8 * max((int)data->chromagram[i] - (int)mean, 0);
                g = 0;
                message.clear();
                message.push_back( 240 );
                message.push_back( 0 );
                message.push_back( 32 );
                message.push_back( 41);
                message.push_back( 2 );
                message.push_back( 16 );
                message.push_back( 11);
                message.push_back( k );
                message.push_back( r );
                message.push_back( g );
                message.push_back( b );
                message.push_back( 247 );

                midiout->sendMessage( &message );
            }
            printf("%3d, ", b);

        }
        printf("\n");
    }

    return finished;
}

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/


/*******************************************************************/
int main(int argc, char **argv)
{
    PaStreamParameters  inputParameters;
    PaStream*           stream;
    PaError             err = paNoError;
    paTestData          data;
    int                 i;
    int                 totalFrames;
    int                 numSamples;
    int                 numBytes;

    printf("patest_record.c\n"); fflush(stdout);


    midiout = new RtMidiOut();

    // Check available ports.
    unsigned int nPorts = midiout->getPortCount();
    if ( nPorts == 0 ) {
      std::cout << "No ports available!\n";
    }
    // Open first available port.
    midiout->openPort( 1 );

    c = new Chromagram(FRAMES_PER_BUFFER,SAMPLE_RATE);
    cd = new ChordDetector();

    data.maxFrameIndex = totalFrames = NUM_SECONDS * SAMPLE_RATE; /* Record for a few seconds. */
    data.frameIndex = 0;
    numSamples = totalFrames * NUM_CHANNELS;
    numBytes = numSamples * sizeof(SAMPLE);
    data.recordedSamples = (SAMPLE *) malloc( numBytes ); /* From now on, recordedSamples is initialised. */
    if( data.recordedSamples == NULL )
    {
        printf("Could not allocate record array.\n");
        goto done;
    }
    for( i=0; i<numSamples; i++ ) data.recordedSamples[i] = 0;

    err = Pa_Initialize();
    if( err != paNoError ) goto done;

    inputParameters.device = stoi(argv[1]); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        goto done;
    }
    const   PaDeviceInfo *deviceInfo;
    int numDevices;

    numDevices = Pa_GetDeviceCount();
    for( i=0; i<numDevices; i++ )
    {
            deviceInfo = Pa_GetDeviceInfo( i );
    }
    inputParameters.channelCount = 2;                    /* stereo input */
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    /* Record some audio. -------------------------------------------- */
    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              NULL,                  /* &outputParameters, */
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              recordCallback,
              &data );
    if( err != paNoError ) goto done;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto done;
    printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

    while( ( err = Pa_IsStreamActive( stream ) ) == 1)
    {
        Pa_Sleep(200);

    }

    if( err < 0 ) goto done;

    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto done;

done:
    Pa_Terminate();
    if( data.recordedSamples )       /* Sure it is NULL or valid. */
        free( data.recordedSamples );
    if( err != paNoError )
    {
        fprintf( stderr, "An error occured while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        err = 1;          /* Always return 0 or 1, but no other return codes. */
    }
    delete c;
    delete midiout;
    return err;
}
