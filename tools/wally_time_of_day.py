# Wally's Journey: time-of-day wild tables (owner request 2026-10-26).
#
# The expansion has four times: MORNING 6-10, DAY 10-19, EVENING 19-20, NIGHT 20-6.
# The plain table in the spawn design is the DAY table. For outdoor areas (routes, ocean
# routes, towns and cities) the builder derives MORNING / EVENING / NIGHT tables from it:
#   1. every species' design weight is multiplied by its activity pattern below,
#   2. nocturnal / morning species that fit the area are added for those times,
#   3. the result is re-normalised the usual way (rare species keep their exact %).
# Caves, buildings and underwater areas are not affected by the time of day.
# Sources: canonical time-of-day tables (HeartGold/SoulSilver, Diamond/Pearl/Platinum,
# Sun/Moon, Scarlet/Violet) and Pokédex text about when a species is active.

TIMES = ('MORNING', 'DAY', 'EVENING', 'NIGHT')

# multipliers for (MORNING, DAY, EVENING, NIGHT)
PATTERNS = {
    'ANY':          (1, 1, 1, 1),
    'DAY_ONLY':     (1, 1, 0.5, 0),
    'DAY_LEAN':     (1, 1, 1, 0.3),
    'MORNING_ONLY': (1, 0, 0, 0),
    'MORNING_LEAN': (2, 1, 0.5, 0.3),
    'EVENING_LEAN': (0.5, 0.5, 2, 1),
    'NIGHT_LEAN':   (0.3, 0.3, 1, 2),
    'NIGHT_ONLY':   (0, 0, 0.5, 1),
}

ACTIVITY = {
    # ---- active at night
    'HOOTHOOT': 'NIGHT_ONLY', 'NOCTOWL': 'NIGHT_ONLY',          # HGSS: night only
    'SPINARAK': 'NIGHT_ONLY', 'ARIADOS': 'NIGHT_ONLY',          # HGSS: night only
    'MURKROW': 'NIGHT_ONLY', 'MISDREAVUS': 'NIGHT_ONLY',        # HGSS: night only
    'GASTLY': 'NIGHT_ONLY', 'HAUNTER': 'NIGHT_LEAN',            # outdoors only at night
    'VENONAT': 'NIGHT_ONLY', 'VENOMOTH': 'NIGHT_ONLY',          # HGSS: night only
    'ODDISH': 'NIGHT_ONLY', 'GLOOM': 'NIGHT_LEAN',              # HGSS night; buries itself by day
    'HOUNDOUR': 'NIGHT_LEAN', 'MEOWTH': 'NIGHT_LEAN',           # HGSS: night
    'RATTATA_ALOLA': 'NIGHT_ONLY',                               # Alola: night only
    'ZUBAT': 'NIGHT_ONLY',                                       # outdoors only at night (caves: any)
    'VOLBEAT': 'NIGHT_LEAN', 'ILLUMISE': 'NIGHT_LEAN',          # glow and dance at night
    'DUSKULL': 'NIGHT_LEAN', 'SHUPPET': 'NIGHT_LEAN',           # wander at night (dex)
    'MORELULL': 'NIGHT_LEAN', 'NICKIT': 'NIGHT_LEAN',
    'PUMPKABOO': 'NIGHT_LEAN', 'GREAVARD': 'NIGHT_LEAN',
    'STARYU': 'NIGHT_LEAN', 'CHINCHOU': 'NIGHT_LEAN', 'LANTURN': 'NIGHT_LEAN', 'LUMINEON': 'NIGHT_LEAN',
    'KRICKETOT': 'EVENING_LEAN', 'ROCKRUFF': 'EVENING_LEAN',
    # ---- active in the morning / by day
    'LEDYBA': 'MORNING_ONLY',                                    # HGSS: morning only
    'SENTRET': 'DAY_ONLY', 'CATERPIE': 'DAY_ONLY', 'WEEDLE': 'DAY_ONLY',   # HGSS: not at night
    'YUNGOOS': 'DAY_ONLY',                                       # Alola: day only
    'SUNKERN': 'DAY_ONLY', 'HELIOPTILE': 'DAY_ONLY', 'FOMANTIS': 'DAY_ONLY',
    'PIDGEY': 'DAY_LEAN', 'SPEAROW': 'DAY_LEAN', 'TAILLOW': 'DAY_LEAN', 'SWELLOW': 'DAY_LEAN',
    'STARLY': 'DAY_LEAN', 'PIDOVE': 'DAY_LEAN', 'FLETCHLING': 'DAY_LEAN', 'ROOKIDEE': 'DAY_LEAN',
    'PIKIPEK': 'DAY_LEAN', 'NATU': 'DAY_LEAN', 'XATU': 'DAY_LEAN', 'HOPPIP': 'DAY_LEAN',
    'YANMA': 'DAY_LEAN', 'AIPOM': 'DAY_LEAN', 'COMBEE': 'MORNING_LEAN', 'CHERUBI': 'DAY_LEAN',
    'BUDEW': 'DAY_LEAN', 'ROSELIA': 'DAY_LEAN', 'CUTIEFLY': 'DAY_LEAN', 'SWABLU': 'DAY_LEAN',
    'DUCKLETT': 'DAY_LEAN', 'LILLIPUP': 'DAY_LEAN',
}

FOREST = ('ROUTE102', 'ROUTE104', 'ROUTE116', 'ROUTE117', 'ROUTE119', 'ROUTE120',
          'ROUTE121', 'ROUTE123', 'PETALBURG_WOODS', 'SAFARI_ZONE_NORTH', 'SAFARI_ZONE_NORTHWEST')
OPEN_GRASS = ('ROUTE101', 'ROUTE103', 'ROUTE110', 'ROUTE112', 'ROUTE113', 'ROUTE114',
              'ROUTE115', 'ROUTE118', 'SAFARI_ZONE_SOUTH', 'SAFARI_ZONE_SOUTHWEST',
              'SAFARI_ZONE_SOUTHEAST', 'SAFARI_ZONE_NORTHEAST')
NEAR_CAVES = ('ROUTE111', 'ROUTE112', 'ROUTE114', 'ROUTE115', 'ROUTE116', 'JAGGED_PASS')

# Extra species for a time of day, by area: {area: {time: {species: design weight}}}
ADDITIONS = {}
def _add(areas, time, **sp):
    for a in areas:
        ADDITIONS.setdefault(a, {}).setdefault(time, {}).update(sp)

_add(FOREST, 'NIGHT', HOOTHOOT=10, SPINARAK=10)
_add(OPEN_GRASS, 'NIGHT', HOOTHOOT=10)
_add(NEAR_CAVES, 'NIGHT', ZUBAT=10)
_add(('MT_PYRE_EXTERIOR', 'MT_PYRE_SUMMIT'), 'NIGHT', GASTLY=10, MURKROW=10)
_add(('ROUTE119', 'ROUTE120', 'ROUTE123'), 'NIGHT', VENONAT=5)
_add(('ROUTE102', 'ROUTE104', 'ROUTE116', 'ROUTE117', 'PETALBURG_WOODS'), 'MORNING', LEDYBA=10)

OUTDOOR_TYPES = ('MAP_TYPE_ROUTE', 'MAP_TYPE_OCEAN_ROUTE', 'MAP_TYPE_CITY', 'MAP_TYPE_TOWN')


def derive(area, kind, spec, time):
    """Table for one time of day from the DAY design table. kind: 'land' or 'water'.
    Returns (lo, hi, {species: weight}) or None when it would equal the DAY table."""
    lo, hi, weights = spec
    ti = TIMES.index(time)
    out = {}
    changed = False
    for sp, w in weights.items():
        key = sp.strip()
        m = PATTERNS[ACTIVITY.get(key, 'ANY')][ti]
        if m != 1:
            changed = True
        nw = w * m
        if w <= 1:                      # rare species never become more common
            nw = min(nw, w)
        if nw > 0:
            out[key] = out.get(key, 0) + nw
    if kind == 'land':
        extra = dict(ADDITIONS.get(area, {}).get(time, {}))
        if time == 'EVENING':            # dusk: night visitors start to appear, at half weight
            for sp, w in ADDITIONS.get(area, {}).get('NIGHT', {}).items():
                extra.setdefault(sp, w * 0.5)
        for sp, w in extra.items():
            if sp not in out:
                out[sp] = w
                changed = True
    if not changed or not out:
        return None
    return (lo, hi, out)
