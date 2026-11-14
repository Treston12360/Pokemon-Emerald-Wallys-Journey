#ifndef GUARD_CONSTANTS_SPEAKER_NAMES_H
#define GUARD_CONSTANTS_SPEAKER_NAMES_H

enum SpeakerNames {
    SP_NAME_NONE = 0,
    SP_NAME_MOM,
    SP_NAME_PLAYER,
    // Project ARIA (owner 2026-11-14: every important character gets the name box)
    SP_NAME_BIRCH,
    SP_NAME_WALLY,
    SP_NAME_OAK,
    SP_NAME_VARNELL,
    SP_NAME_BLUE,
    SP_NAME_BILL,
    SP_NAME_LANCE,
    SP_NAME_KOGA,
    SP_NAME_MIGUEL,
    SP_NAME_KIYOSHI,
    SP_NAME_FUJI,
    // ENG-NAMEBOX Hoenn (2026-11-14). charmap.txt has a matching SPK_* byte for each name below, used as
    // {SPEAKER SPK_X} inside text. KEEP THE TWO IN THE SAME ORDER (charmap value = position in this enum).
    SP_NAME_TATE,
    SP_NAME_LIZA,
    SP_NAME_TATE_LIZA,
    SP_NAME_MAY,
    SP_NAME_BRENDAN,
    SP_NAME_STEVEN,
    SP_NAME_DAD,
    SP_NAME_NORMAN,
    SP_NAME_SCOTT,
    SP_NAME_BRINEY,
    SP_NAME_ARCHIE,
    SP_NAME_MAXIE,
    SP_NAME_WATTSON,
    SP_NAME_WALLACE,
    SP_NAME_JUAN,
    SP_NAME_ROXANNE,
    SP_NAME_STERN,
    SP_NAME_WINONA,
    SP_NAME_FLANNERY,
    SP_NAME_BRAWLY,
    SP_NAME_MR_STONE,
    SP_NAME_COZMO,
    SP_NAME_SIDNEY,
    SP_NAME_PHOEBE,
    SP_NAME_GLACIA,
    SP_NAME_DRAKE,
    SP_NAME_TABITHA,
    SP_NAME_MATT,
    SP_NAME_COURTNEY,
    SP_NAME_SHELLY,
    SP_NAME_RIVAL,   // May or Brendan, whichever is the player's rival (for texts both rivals share)
    SP_NAME_COUNT
};

#endif // GUARD_CONSTANTS_SPEAKER_NAMES_H
