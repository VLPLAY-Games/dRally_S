#ifndef SWITCH_AUDIO_H
#define SWITCH_AUDIO_H

/* Nintendo Switch audio output through the audio renderer (audren).

   The game fills 16-bit stereo buffers from a callback, as SDL's audio callback model does:
   SwitchAudio_open starts a thread that calls the callback for each buffer and queues it on one
   audren voice. The voice plays at the sample rate the game asks for. */

typedef void (*SwitchAudioCallback)(void * userdata, unsigned char * stream, int len);

/* Returns 0 on success, -1 when audren is not available (the caller falls back to SDL audio). */
int SwitchAudio_open(int freq, int samples, SwitchAudioCallback callback, void * userdata);
void SwitchAudio_pause(int paused);
int SwitchAudio_isOpen(void);
void SwitchAudio_close(void);

#endif // SWITCH_AUDIO_H
