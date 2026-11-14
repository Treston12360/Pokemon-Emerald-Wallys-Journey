MP3 music for Wally's Journey (Windows game folder: this is the "music" folder next to WallysJourney.exe)

Put an MP3 here named after the song it replaces, e.g.
    MUS_LITTLEROOT.mp3        replaces Littleroot Town's music
    MUS_RG_PALLET.mp3         replaces Pallet Town's music
The names are the MUS_ names in include/constants/songs.h (a map's song is its "music" in data/maps/*/map.json).
Only background music can be replaced (MUS_ songs), not sound effects or cries.
No MP3 for a song = the original music plays, exactly as before.

Optional settings file with the same name, e.g. MUS_LITTLEROOT.txt:
    loop_start=12.5     seconds: where the song jumps back to when it reaches the end (default 0)
    loop_end=95.25      seconds: where it jumps back from (default: the end of the file)
    volume=0.3          loudness. Default 0.15 puts a normal MP3 at about the level of the game's music.

The MP3 follows the game: it pauses for fanfares, fades with the music, and stops when the song changes.
