// ENG-MP3 glue between the GBA sound engine and mp3_music.c (rules for the files are in mp3_music.c).
//
// Every sound frame, before mixing: look at what the background-music player (gMPlayInfo_BGM) is playing.
// If that song has an MP3, the MP3 plays and the GBA song is muted (it still runs, so the game sees normal
// music state). The MP3 follows the GBA player: paused while it is paused (fanfares), quieter while it
// fades or while the game turns the music down (e.g. during a cry), stopped when the song changes.
#include "global.h"
#include "gba/m4a_internal.h"
#include "sound_mixer.h"
#include "mp3_music.h"
#include "mp3_song_names.h"

extern struct MusicPlayerInfo gMPlayInfo_BGM;
extern struct MusicPlayerTrack gMPlayTrack_BGM[10];

u8 gCgbMuteMask; // read by cgb_audio_generate: bit i set = GB channel i+1 is silent

static const struct SongHeader *sLastHeader;
static bool8 sMp3On;

static int FindSong(const struct SongHeader *header)
{
    u32 n;
    for (n = 0; n < ARRAY_COUNT(sMp3SongNames); n++)
    {
        if (sMp3SongNames[n] != NULL && gSongTable[n].header == header)
            return n;
    }
    return -1;
}

static bool32 IsBgmTrack(const void *track)
{
    const u8 *t = track;
    return t >= (const u8 *)&gMPlayTrack_BGM[0] && t < (const u8 *)&gMPlayTrack_BGM[ARRAY_COUNT(gMPlayTrack_BGM)];
}

// RunMixerFrame, before the GBA channels are mixed
void Mp3Glue_BeforeMix(struct SoundMixerState *mixer)
{
    const struct SongHeader *header = gMPlayInfo_BGM.songHeader;
    u32 i;

    if (header != sLastHeader)
    {
        int n = header ? FindSong(header) : -1;
        sLastHeader = header;
        sMp3On = (n >= 0) ? Mp3Music_Start(sMp3SongNames[n], n) : FALSE;
        if (!sMp3On)
            Mp3Music_Stop();
    }
    gCgbMuteMask = 0;
    if (sMp3On && mixer->cgbChans != NULL)
    {
        for (i = 0; i < 4; i++)
        {
            if (IsBgmTrack(mixer->cgbChans[i].track))
                gCgbMuteMask |= 1 << i;
        }
    }
}

// SampleMixer: TRUE = do not add this channel to the output (it belongs to the MP3-replaced song)
bool32 Mp3Glue_MuteChannel(const struct MixerSource *chan)
{
    return sMp3On && IsBgmTrack(chan->track);
}

// m4aSoundVSync: add the MP3 to the finished frame (interleaved stereo floats)
void Mp3Glue_AddToFrame(float *out, s32 frames)
{
    float vol = 1.0f;
    u32 status = gMPlayInfo_BGM.status;
    u32 i;

    if (!sMp3On)
        return;
    if (!Mp3Music_IsPlaying())
    {
        sMp3On = FALSE;
        return;
    }
    if ((status & MUSICPLAYER_STATUS_TRACK) == 0 || (status & MUSICPLAYER_STATUS_PAUSE))
        return; // stopped or paused: hold the place
    for (i = 0; i < gMPlayInfo_BGM.trackCount && i < ARRAY_COUNT(gMPlayTrack_BGM); i++)
    {
        if (gMPlayTrack_BGM[i].flags & MPT_FLG_EXIST)
        {
            vol = gMPlayTrack_BGM[i].volX / 64.0f; // 64 = normal; fades and m4aMPlayVolumeControl both set volX
            break;
        }
    }
    if (vol > 1.0f)
        vol = 1.0f;
    Mp3Music_Mix(out, frames, 42048, vol);
}
