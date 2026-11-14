#ifndef GUARD_CONSTANTS_TMS_HMS_H
#define GUARD_CONSTANTS_TMS_HMS_H

#define FOREACH_TM(F) \
    F(FOCUS_PUNCH) \
    F(DRAGON_CLAW) \
    F(WATER_PULSE) \
    F(CALM_MIND) \
    F(ROAR) \
    F(TOXIC) \
    F(HAIL) \
    F(BULK_UP) \
    F(BULLET_SEED) \
    F(HIDDEN_POWER) \
    F(SUNNY_DAY) \
    F(TAUNT) \
    F(ICE_BEAM) \
    F(BLIZZARD) \
    F(HYPER_BEAM) \
    F(LIGHT_SCREEN) \
    F(PROTECT) \
    F(RAIN_DANCE) \
    F(GIGA_DRAIN) \
    F(SAFEGUARD) \
    F(FRUSTRATION) \
    F(SOLAR_BEAM) \
    F(IRON_TAIL) \
    F(THUNDERBOLT) \
    F(THUNDER) \
    F(EARTHQUAKE) \
    F(RETURN) \
    F(DIG) \
    F(PSYCHIC) \
    F(SHADOW_BALL) \
    F(BRICK_BREAK) \
    F(DOUBLE_TEAM) \
    F(REFLECT) \
    F(SHOCK_WAVE) \
    F(FLAMETHROWER) \
    F(SLUDGE_BOMB) \
    F(SANDSTORM) \
    F(FIRE_BLAST) \
    F(ROCK_TOMB) \
    F(AERIAL_ACE) \
    F(TORMENT) \
    F(FACADE) \
    F(SECRET_POWER) \
    F(REST) \
    F(ATTRACT) \
    F(THIEF) \
    F(STEEL_WING) \
    F(SKILL_SWAP) \
    F(SNATCH) \
    F(OVERHEAT) \
    /* TM51-TM100: Project ARIA Kanto TMs (owner approved 2026-11-14, docs/KANTO_TM_LIST.md). Kanto gives  */ \
    /* only these, because all 50 Gen 3 TMs are already found in Hoenn. Item IDs were unused before.       */ \
    F(STEALTH_ROCK) \
    F(SCALD) \
    F(VOLT_SWITCH) \
    F(ENERGY_BALL) \
    F(VENOSHOCK) \
    F(PSYSHOCK) \
    F(WILL_O_WISP) \
    F(DRAGON_PULSE) \
    F(SWORDS_DANCE) \
    F(NASTY_PLOT) \
    F(SUBSTITUTE) \
    F(THUNDER_WAVE) \
    F(AGILITY) \
    F(SLEEP_TALK) \
    F(TRICK_ROOM) \
    F(TAILWIND) \
    F(ENCORE) \
    F(FLASH_CANNON) \
    F(DARK_PULSE) \
    F(EARTH_POWER) \
    F(FOCUS_BLAST) \
    F(STONE_EDGE) \
    F(TRICK) \
    F(U_TURN) \
    F(KNOCK_OFF) \
    F(DAZZLING_GLEAM) \
    F(X_SCISSOR) \
    F(POISON_JAB) \
    F(ROCK_SLIDE) \
    F(AIR_SLASH) \
    F(BUG_BUZZ) \
    F(ZEN_HEADBUTT) \
    F(SEED_BOMB) \
    F(WILD_CHARGE) \
    F(IRON_HEAD) \
    F(DRAIN_PUNCH) \
    F(HYPER_VOICE) \
    F(PLAY_ROUGH) \
    F(GRASS_KNOT) \
    F(HEAT_WAVE) \
    F(HURRICANE) \
    F(BULLDOZE) \
    F(LIQUIDATION) \
    F(SHADOW_CLAW) \
    F(PSYCHO_CUT) \
    F(SLUDGE_WAVE) \
    F(ROOST) \
    F(DRAGON_DANCE) \
    F(BODY_PRESS) \
    F(SPIKES)

#define FOREACH_HM(F) \
    F(CUT) \
    F(FLY) \
    F(SURF) \
    F(STRENGTH) \
    F(FLASH) \
    F(ROCK_SMASH) \
    F(WATERFALL) \
    F(DIVE)

#define FOREACH_TMHM(F) \
    FOREACH_TM(F) \
    FOREACH_HM(F)

#endif
