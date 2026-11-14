// ENG-MP3 (Project ARIA, owner 2026-11-14): any background song can be replaced by an MP3 file.
//
// Drop  music/MUS_NAME.mp3  (the song's constant name from include/constants/songs.h, e.g.
// music/MUS_LITTLEROOT.mp3) next to the game. When the game starts that song, the MP3 plays instead and the
// GBA version of the song keeps running silently underneath, so fades, pauses (fanfares), battle music
// swaps and every "is the music still playing?" check behave exactly as before.
//
// Optional  music/MUS_NAME.txt  (one setting per line):
//     loop_start=12.5     seconds; where the song jumps back to (default 0 = the whole file loops)
//     loop_end=95.25      seconds; where it jumps from (default = end of file)
//     volume=0.3          1.0 = as loud as the file. Default 0.15: a normal MP3 is much louder than the
//                         GBA music mix, 0.15 puts a typical track at about the same level.
//
// This file has no game headers on purpose: dr_mp3 (public domain / MIT-0, v0.7.4, src/platform/dr_mp3.h)
// is compiled here and only this small API is shared with the sound code (mp3_music.h).

#define DR_MP3_IMPLEMENTATION
#define DR_MP3_NO_STDIO_WARNINGS
#include "dr_mp3.h"
#include "mp3_music.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_FRAMES 1024

static struct {
    int open;
    drmp3 mp3;
    char name[64];
    float volume;
    unsigned long long loopStart, loopEnd; // in source frames; loopEnd 0 = end of file
    unsigned long long readPos;            // next source frame the decoder will give
    float chunk[(CHUNK_FRAMES + 1) * 2];
    int chunkFrames;                       // valid frames in chunk
    double frac;                           // read position inside chunk (source frames)
} sM;

// Remembers which songs have no MP3, so the disk is checked only once per song.
static unsigned char sMissing[1024];

static void ReadSettings(const char *name, unsigned rate)
{
    char path[160];
    FILE *f;
    char line[128];

    sM.volume = 0.15f;
    sM.loopStart = sM.loopEnd = 0;
    snprintf(path, sizeof(path), "music/%s.txt", name);
    f = fopen(path, "r");
    if (f == NULL)
        return;
    while (fgets(line, sizeof(line), f))
    {
        double v;
        if (sscanf(line, " loop_start = %lf", &v) == 1 && v >= 0)
            sM.loopStart = (unsigned long long)(v * rate);
        else if (sscanf(line, " loop_end = %lf", &v) == 1 && v > 0)
            sM.loopEnd = (unsigned long long)(v * rate);
        else if (sscanf(line, " volume = %lf", &v) == 1 && v >= 0)
            sM.volume = (float)v;
    }
    fclose(f);
}

void Mp3Music_Stop(void)
{
    if (sM.open)
        drmp3_uninit(&sM.mp3);
    sM.open = 0;
    sM.name[0] = 0;
}

int Mp3Music_Start(const char *songName, int songNum)
{
    char path[160];

    if (sM.open && strcmp(sM.name, songName) == 0)
        return 1; // same song asked again: keep playing where it is
    Mp3Music_Stop();
    if (songNum >= 0 && songNum < (int)sizeof(sMissing) && sMissing[songNum])
        return 0;
    snprintf(path, sizeof(path), "music/%s.mp3", songName);
    if (!drmp3_init_file(&sM.mp3, path, NULL))
    {
        if (songNum >= 0 && songNum < (int)sizeof(sMissing))
            sMissing[songNum] = 1;
        return 0;
    }
    sM.open = 1;
    snprintf(sM.name, sizeof(sM.name), "%s", songName);
    ReadSettings(songName, sM.mp3.sampleRate);
    sM.readPos = 0;
    sM.chunkFrames = 0;
    sM.frac = 0;
    return 1;
}

int Mp3Music_IsPlaying(void)
{
    return sM.open;
}

// Reads the next chunk of source frames (stereo float), handling the loop.
static int FillChunk(void)
{
    // keep the last frame so interpolation across chunks is smooth
    float last[2] = {0, 0};
    unsigned long long want = CHUNK_FRAMES;
    drmp3_uint64 got;
    int tries = 0;

    if (sM.chunkFrames > 0)
    {
        last[0] = sM.chunk[(sM.chunkFrames - 1) * 2];
        last[1] = sM.chunk[(sM.chunkFrames - 1) * 2 + 1];
    }
    for (;;)
    {
        if (sM.loopEnd && sM.readPos + want > sM.loopEnd)
            want = sM.loopEnd > sM.readPos ? sM.loopEnd - sM.readPos : 0;
        got = 0;
        if (want)
        {
            float tmp[CHUNK_FRAMES * 2];
            int ch = sM.mp3.channels;
            if (ch == 2)
                got = drmp3_read_pcm_frames_f32(&sM.mp3, want, sM.chunk + 2);
            else
            {
                float mono[CHUNK_FRAMES * 2];
                drmp3_uint64 i;
                got = drmp3_read_pcm_frames_f32(&sM.mp3, want, ch == 1 ? mono : tmp);
                for (i = 0; i < got; i++)
                {
                    float l = ch == 1 ? mono[i] : tmp[i * ch];
                    float r = ch == 1 ? mono[i] : tmp[i * ch + 1];
                    sM.chunk[2 + i * 2] = l;
                    sM.chunk[2 + i * 2 + 1] = r;
                }
            }
        }
        if (got > 0)
        {
            sM.readPos += got;
            break;
        }
        // end of the song (or of its loop): jump back
        if (++tries > 2 || !drmp3_seek_to_pcm_frame(&sM.mp3, sM.loopStart))
            return 0;
        sM.readPos = sM.loopStart;
        want = CHUNK_FRAMES;
    }
    sM.chunk[0] = last[0];
    sM.chunk[1] = last[1];
    sM.chunkFrames = (int)got + 1; // index 0 = previous frame
    return 1;
}

void Mp3Music_Mix(float *out, int frames, int outRate, float gameVolume)
{
    double step;
    int i;
    float vol;

    if (!sM.open)
        return;
    step = (double)sM.mp3.sampleRate / outRate;
    vol = sM.volume * gameVolume;
    for (i = 0; i < frames; i++)
    {
        int idx;
        float t, l, r;
        while (sM.chunkFrames < 2 || sM.frac >= sM.chunkFrames - 1)
        {
            if (sM.chunkFrames >= 2)
                sM.frac -= sM.chunkFrames - 1;
            if (!FillChunk())
            {
                Mp3Music_Stop();
                return;
            }
        }
        idx = (int)sM.frac;
        t = (float)(sM.frac - idx);
        l = sM.chunk[idx * 2] + (sM.chunk[idx * 2 + 2] - sM.chunk[idx * 2]) * t;
        r = sM.chunk[idx * 2 + 1] + (sM.chunk[idx * 2 + 3] - sM.chunk[idx * 2 + 1]) * t;
        out[i * 2] += l * vol;
        out[i * 2 + 1] += r * vol;
        sM.frac += step;
    }
}
