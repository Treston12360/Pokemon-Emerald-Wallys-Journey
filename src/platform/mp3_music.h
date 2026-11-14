#ifndef GUARD_MP3_MUSIC_H
#define GUARD_MP3_MUSIC_H

// ENG-MP3: background songs from music/MUS_NAME.mp3 (see mp3_music.c for the file rules).
int Mp3Music_Start(const char *songName, int songNum); // 1 = an MP3 is playing for this song
void Mp3Music_Stop(void);
int Mp3Music_IsPlaying(void);
// Adds `frames` stereo frames at outRate into out (interleaved L,R), scaled by gameVolume (0..1).
void Mp3Music_Mix(float *out, int frames, int outRate, float gameVolume);

#endif
