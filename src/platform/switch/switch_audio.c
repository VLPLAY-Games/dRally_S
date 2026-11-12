#include "switch_audio.h"

#include <switch.h>
#include <malloc.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

/* Why audren and not SDL's audout driver: on the Switch the old audout service produced audible
   noise on loud low tones, the audio renderer does not. The layout follows what RetroArch does:
   the wave buffers all address the whole memory pool, the sample offsets pick each slice, and the
   voice is started once the first buffer is queued. */

#define NUM_BUFFERS 3
#define ALIGN 0x1000

static struct {
    int                 open;
    volatile int        paused;
    volatile int        quit;
    int                 bytes;      /* bytes of audio per buffer */
    unsigned int        aligned;    /* buffer size rounded up to the page size */
    SwitchAudioCallback callback;
    void *              userdata;
    AudioDriver         drv;
    AudioDriverWaveBuf  wave[NUM_BUFFERS];
    void *              pool;
    int                 driver_open;
    pthread_t           thread;
    int                 thread_started;
} A = {0};

static int wave_ready(const AudioDriverWaveBuf * w){

    return (w->state == AudioDriverWaveBufState_Done) || (w->state == AudioDriverWaveBufState_Free);
}

static void submit(int i, int silence){

    AudioDriverWaveBuf * w = &A.wave[i];
    unsigned char * dst = (unsigned char *)A.pool+(size_t)i*A.aligned;

    memset(dst, 0, A.bytes);
    if(!silence && A.callback) A.callback(A.userdata, dst, A.bytes);

    /* the audio renderer reads the memory by DMA, not through the CPU cache */
    armDCacheFlush(dst, A.aligned);
    w->start_sample_offset = (s32)((size_t)i*A.aligned/4);
    w->end_sample_offset = w->start_sample_offset+A.bytes/4;
    audrvVoiceAddWaveBuf(&A.drv, 0, w);
    audrvUpdate(&A.drv);
    if(!audrvVoiceIsPlaying(&A.drv, 0)) audrvVoiceStart(&A.drv, 0);
}

static void * audio_thread(void * arg){

    int next = 0;

    (void)arg;

    /* keep the mixer off the core the game itself runs on */
    svcSetThreadCoreMask(CUR_THREAD_HANDLE, 2, 1u<<2);

    while(!A.quit){

        int n = 0;

        audrvUpdate(&A.drv);

        while((n < NUM_BUFFERS) && wave_ready(&A.wave[(next+n)%NUM_BUFFERS])) n++;

        /* normally one buffer is back; after a stall several are, and the rest is refilled too */
        while(n-- > 0){

            submit(next, A.paused);
            next = (next+1)%NUM_BUFFERS;
        }

        audrenWaitFrame();
    }

    return NULL;
}

int SwitchAudio_open(int freq, int samples, SwitchAudioCallback callback, void * userdata){

    static const AudioRendererConfig config = {
        .output_rate = AudioRendererOutputRate_48kHz,
        .num_voices = 24,
        .num_effects = 0,
        .num_sinks = 1,
        .num_mix_objs = 1,
        .num_mix_buffers = 2,
    };
    static const u8 sink_channels[] = { 0, 1 };
    pthread_attr_t attr;
    int i, j, pool_id;

    if(A.open) return 0;

    memset(&A, 0, sizeof(A));
    A.bytes = samples*4;
    A.aligned = ((unsigned int)A.bytes+ALIGN-1)&~(ALIGN-1);
    A.callback = callback;
    A.userdata = userdata;
    A.paused = 1;   /* the game un-pauses it when it has sound to play */

    if(R_FAILED(audrenInitialize(&config))) return -1;
    if(R_FAILED(audrvCreate(&A.drv, &config, 2))){

        audrenExit();
        return -1;
    }
    A.driver_open = 1;

    A.pool = memalign(ALIGN, (size_t)A.aligned*NUM_BUFFERS);
    if(!A.pool){

        SwitchAudio_close();
        return -1;
    }
    memset(A.pool, 0, (size_t)A.aligned*NUM_BUFFERS);
    armDCacheFlush(A.pool, (size_t)A.aligned*NUM_BUFFERS);
    pool_id = audrvMemPoolAdd(&A.drv, A.pool, (size_t)A.aligned*NUM_BUFFERS);
    audrvMemPoolAttach(&A.drv, pool_id);
    audrvDeviceSinkAdd(&A.drv, AUDREN_DEFAULT_DEVICE_NAME, 2, sink_channels);

    if(R_FAILED(audrvUpdate(&A.drv)) || R_FAILED(audrenStartAudioRenderer())){

        SwitchAudio_close();
        return -1;
    }

    /* the voice resamples from the game's rate to the renderer's 48 kHz */
    audrvVoiceInit(&A.drv, 0, 2, PcmFormat_Int16, freq);
    audrvVoiceSetDestinationMix(&A.drv, 0, AUDREN_FINAL_MIX_ID);
    for(i = 0; i < 2; i++){

        for(j = 0; j < 2; j++) audrvVoiceSetMixFactor(&A.drv, 0, (i == j) ? 1.0f : 0.0f, i, j);
    }

    for(i = 0; i < NUM_BUFFERS; i++){

        memset(&A.wave[i], 0, sizeof(A.wave[i]));
        A.wave[i].data_raw = A.pool;
        A.wave[i].size = (u64)A.aligned*NUM_BUFFERS;
    }
    audrvUpdate(&A.drv);

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 256*1024);
    if(pthread_create(&A.thread, &attr, audio_thread, NULL) != 0){

        pthread_attr_destroy(&attr);
        SwitchAudio_close();
        return -1;
    }
    pthread_attr_destroy(&attr);
    A.thread_started = 1;
    A.open = 1;

    printf("[dRally.SOUND] audren: %d Hz, %d bytes x %d buffers\n", freq, A.bytes, NUM_BUFFERS);
    return 0;
}

void SwitchAudio_pause(int paused){

    A.paused = !!paused;
}

int SwitchAudio_isOpen(void){

    return A.open;
}

void SwitchAudio_close(void){

    if(A.thread_started){

        A.quit = 1;
        pthread_join(A.thread, NULL);
        A.thread_started = 0;
    }

    if(A.driver_open){

        audrvVoiceStop(&A.drv, 0);
        audrvUpdate(&A.drv);
        audrvClose(&A.drv);
        audrenExit();
        A.driver_open = 0;
    }

    free(A.pool);
    A.pool = NULL;
    A.open = 0;
}
