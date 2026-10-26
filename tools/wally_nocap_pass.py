# Wally's Journey: clean-up pass after the per-table slot limit was removed (2026-10-26).
# Applied to the design dict D at the end of tools/wally_spawn_redesign.py.
#
# Rule: anything at 1% or less is a hidden rare visitor (not shown in the DexNav list or on the
# Pokedex area map until caught; counted in the DexNav "RARE x of y found" box). Anything above 1%
# is a normal table entry. The old 12-slot land layout forced two 1% "tail" slots in every table;
# where those were filled with ordinary wildlife, it now gets a real uncommon rate instead.

SPECIAL = {
    # starters
    'BULBASAUR', 'CHARMANDER', 'SQUIRTLE', 'CHIKORITA', 'CYNDAQUIL', 'TOTODILE', 'TREECKO', 'TORCHIC',
    'MUDKIP', 'TURTWIG', 'CHIMCHAR', 'PIPLUP', 'SNIVY', 'TEPIG', 'OSHAWOTT', 'CHESPIN', 'FENNEKIN',
    'FROAKIE', 'ROWLET', 'LITTEN', 'POPPLIO', 'GROOKEY', 'SCORBUNNY', 'SOBBLE', 'SPRIGATITO', 'FUECOCO', 'QUAXLY',
    # pseudo-legendary lines
    'DRATINI', 'DRAGONAIR', 'LARVITAR', 'PUPITAR', 'BAGON', 'SHELGON', 'BELDUM', 'METANG', 'GIBLE', 'GABITE',
    'DEINO', 'ZWEILOUS', 'GOOMY', 'SLIGGOO', 'JANGMO_O', 'HAKAMO_O', 'DREEPY', 'DRAKLOAK', 'FRIGIBAX', 'ARCTIBAX',
    # fossils
    'OMANYTE', 'KABUTO', 'AERODACTYL', 'LILEEP', 'ANORITH', 'CRANIDOS', 'SHIELDON', 'TIRTOUGA', 'ARCHEN',
    'TYRUNT', 'AMAURA', 'DRACOZOLT', 'ARCTOZOLT', 'DRACOVISH', 'ARCTOVISH',
    # owner elusive list and famously rare finds
    'SPIRITOMB', 'ROTOM', 'MIMIKYU_DISGUISED', 'POLTEAGEIST', 'KECLEON', 'EEVEE', 'RIOLU', 'DRAMPA',
    'LARVESTA', 'GIMMIGHOUL_ROAMING', 'ZORUA_HISUI', 'MEOWTH_GALAR', 'EISCUE',
    # babies
    'SMOOCHUM', 'TOXEL', 'MUNCHLAX',
}

# Ordinary wildlife that only sat at 1% because it filled one of the last two land slots.
# New rate (percent); the table's common species absorb the difference.
RAISE = {
    'ROUTE101': {'LECHONK': 3, 'PIDGEY': 3},
    'ROUTE102': {'DEERLING_SPRING': 2, 'SKWOVET': 3},
    'ROUTE103': {'SKWOVET': 3},
    'ROUTE104': {'STARLY': 3},
    'ROUTE110': {'PIKACHU': 2},
    'ROUTE111': {'CYCLIZAR': 3, 'KLAWF': 3},
    'ROUTE114': {'SEEDOT': 3, 'STUNKY': 3},
    'ROUTE116': {'PURRLOIN': 3, 'ABRA': 4},
    'ROUTE117': {'BUDEW': 3, 'MARILL': 2},
    'PETALBURG_WOODS': {'PARAS': 3},
    'RUSTURF_TUNNEL': {'NOSEPASS': 3, 'ONIX': 3},
    'GRANITE_CAVE_1F': {'BRONZOR': 3},
    'GRANITE_CAVE_B1F': {'BRONZOR': 3},
    'GRANITE_CAVE_B2F': {'DURANT': 3, 'NOSEPASS': 3},
    'JAGGED_PASS': {'SKARMORY': 3},
    'METEOR_FALLS_STEVENS_CAVE': {'DRUDDIGON': 3},
    'MT_PYRE_1F': {'GOTHITA': 3},
    'CAVE_OF_ORIGIN_ENTRANCE': {'BRONZONG': 3},
    'CAVE_OF_ORIGIN_1F': {'BRONZONG': 3},
    'CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP1': {'BRONZONG': 3},
    'CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP2': {'BRONZONG': 3},
    'CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP3': {'BRONZONG': 3},
    'VICTORY_ROAD_B1F': {'MEDICHAM': 3},
    'MAGMA_HIDEOUT_2F_1R': {'CAMERUPT': 3},
    'MAGMA_HIDEOUT_3F_2R': {'CAMERUPT': 3},
    'MIRAGE_TOWER_3F': {'SIGILYPH': 3},
    'MIRAGE_TOWER_4F': {'SIGILYPH': 3},
    'SAFARI_ZONE_SOUTH': {'PIKACHU': 3},
    'SAFARI_ZONE_SOUTHWEST': {'NIDORINO': 3},
    'SAFARI_ZONE_NORTH': {'APPLIN': 3},
    'SAFARI_ZONE_NORTHWEST': {'SCYTHER': 3},
    'SAFARI_ZONE_SOUTHEAST': {'KANGASKHAN': 4, 'FLAAFFY': 3},
    'SAFARI_ZONE_NORTHEAST': {'HERACROSS': 3},
}

# Evolved Pokemon listed where their level range is below their evolution level: the builder has
# always swapped in the pre-evolution, so the evolved form never appeared. Name what really appears.
PRE_EVO = {
    ('ROUTE111', 'rock', 'GRAVELER'): 'GEODUDE', ('GRANITE_CAVE_B2F', 'rock', 'GRAVELER'): 'GEODUDE',
    ('ROUTE126', 'super', 'DRAGALGE'): 'SKRELP',
    ('MT_PYRE_4F', 'land', 'LAMPENT'): 'LITWICK', ('MT_PYRE_6F', 'land', 'LAMPENT'): 'LITWICK',
    ('MT_PYRE_SUMMIT', 'land', 'DUSCLOPS'): 'DUSKULL',
    ('SEAFLOOR_CAVERN_ROOM4', 'land', 'BARBARACLE'): 'BINACLE',
    ('SEAFLOOR_CAVERN_ROOM5', 'land', 'PALOSSAND'): 'SANDYGAST',
    ('SHOAL_CAVE_LOW_TIDE_LOWER_ROOM', 'land', 'GLALIE'): 'SNORUNT',
    ('MAGMA_HIDEOUT_2F_2R', 'land', 'MAGCARGO'): 'SLUGMA', ('MAGMA_HIDEOUT_3F_1R', 'land', 'MAGCARGO'): 'SLUGMA',
    ('MAGMA_HIDEOUT_3F_3R', 'land', 'MAGCARGO'): 'SLUGMA', ('MAGMA_HIDEOUT_4F', 'land', 'MAGCARGO'): 'SLUGMA',
    ('ABANDONED_SHIP_ROOMS_B1F', 'water', 'JELLICENT'): 'FRILLISH',
    ('ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS', 'water', 'JELLICENT'): 'FRILLISH',
}

# Names that are the same species in the game (form aliases), merged so one Pokemon has one entry.
ALIAS = {'SINISTEA': 'SINISTEA_PHONY', 'MORPEKO': 'MORPEKO_FULL_BELLY', 'PUMPKABOO': 'PUMPKABOO_AVERAGE',
         'DEERLING': 'DEERLING_SPRING', 'FLABEBE': 'FLABEBE_RED', 'BURMY': 'BURMY_PLANT', 'SHELLOS': 'SHELLOS_WEST',
         'WISHIWASHI': 'WISHIWASHI_SOLO', 'EISCUE_ICE': 'EISCUE', 'MIMIKYU': 'MIMIKYU_DISGUISED'}

KINDS = ('land', 'water', 'rock', 'old', 'good', 'super')
LOG = []


def apply(D):
    for area, spec in D.items():
        for kind in KINDS:
            vkey = 'v_' + kind
            table = spec.get(kind)
            visitors = dict(spec.get(vkey, {}))
            if table is None and not visitors:
                continue
            if table is not None:
                lo, hi, w = table
                merged = {}
                for sp, v in w.items():
                    name = sp.strip()
                    name = PRE_EVO.get((area, kind, name), name)
                    if name != sp.strip():
                        LOG.append((area, kind, 'pre-evolution', sp.strip(), name))
                    name = ALIAS.get(name, name)
                    if name in merged:
                        LOG.append((area, kind, 'merged duplicate', name, v))
                    merged[name] = merged.get(name, 0) + v
                for sp, v in RAISE.get(area, {}).items():
                    if sp in merged and merged[sp] <= 1:
                        LOG.append((area, kind, 'raised', sp, f'{merged[sp]:g}% -> {v:g}%'))
                        merged[sp] = v
            else:
                lo = hi = None
                merged = {}
            vis = {}
            for sp, v in visitors.items():
                name = ALIAS.get(sp, sp)
                if v > 1 and table is not None:
                    LOG.append((area, kind, 'visitor into table', name, f'{v:g}%'))
                    merged[name] = merged.get(name, 0) + v
                else:
                    vis[name] = vis.get(name, 0) + v
            for sp in list(merged):
                if merged[sp] <= 1 and (sp in SPECIAL or sp in vis):
                    LOG.append((area, kind, 'table into hidden visitor', sp, f'{merged[sp]:g}%'))
                    vis[sp] = vis.get(sp, 0) + merged.pop(sp)
            for sp in list(merged):
                if merged[sp] <= 1:
                    LOG.append((area, kind, 'still visible at 1% (review)', sp, f'{merged[sp]:g}%'))
            if table is not None:
                spec[kind] = (lo, hi, merged)
            if vis:
                spec[vkey] = vis
            elif vkey in spec:
                del spec[vkey]
    return D
