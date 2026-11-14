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
#include "platform.h"

extern struct MusicPlayerInfo gMPlayInfo_BGM;
extern struct MusicPlayerTrack gMPlayTrack_BGM[10];

u8 gCgbMuteMask; // read by cgb_audio_generate: bit i set = GB channel i+1 is silent
float gCgbGain[4] = {1, 1, 1, 1}; // POL-29: music or sound volume of each GB channel this frame

// POL-29 (Brennan, 2026-11-14): "differentiate music volume and gameplay volume in case you want to turn off music
// but keep gameplay audio". A channel is music when its music player is playing a MUS_ song (background music
// and fanfares); everything else (sound effects, cries) is sound. The main Volume still scales both, in sdl2.c.
static float sMusicGain = 1, sSoundGain = 1;
static bool8 sPlayerIsMusic[8];

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

static bool32 IsMusicTrack(const void *track)
{
    const u8 *t = track;
    u32 p;
    for (p = 0; p < NUM_MUSIC_PLAYERS && p < ARRAY_COUNT(sPlayerIsMusic); p++)
    {
        const u8 *first = (const u8 *)gMPlayTable[p].track;
        if (t >= first && t < first + gMPlayTable[p].numTracks * sizeof(struct MusicPlayerTrack))
            return sPlayerIsMusic[p];
    }
    return FALSE; // cries and anything else: sound
}

// SampleMixer: volume factor for one DirectSound channel
float Audio_ChannelGain(const struct MixerSource *chan)
{
    return IsMusicTrack(chan->track) ? sMusicGain : sSoundGain;
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
    sMusicGain = Platform_GetSetting(PLATFORM_SETTING_MUSIC_VOLUME) / 10.0f;
    sSoundGain = Platform_GetSetting(PLATFORM_SETTING_SOUND_VOLUME) / 10.0f;
    for (i = 0; i < NUM_MUSIC_PLAYERS && i < ARRAY_COUNT(sPlayerIsMusic); i++)
    {
        const struct SongHeader *h = gMPlayTable[i].info->songHeader;
        if (h == NULL)
            sPlayerIsMusic[i] = FALSE;
        else if (i == 0)
            sPlayerIsMusic[i] = TRUE; // the background music player
        else
            sPlayerIsMusic[i] = FindSong(h) >= 0; // a MUS_ song (fanfare) on another player
    }
    gCgbMuteMask = 0;
    for (i = 0; i < 4; i++)
    {
        const void *track = mixer->cgbChans != NULL ? mixer->cgbChans[i].track : NULL;
        gCgbGain[i] = (track != NULL && !IsMusicTrack(track)) ? sSoundGain : sMusicGain;
        if (sMp3On && track != NULL && IsBgmTrack(track))
            gCgbMuteMask |= 1 << i;
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
    Mp3Music_Mix(out, frames, 42048, vol * sMusicGain);
}
