// Wally's Journey: "rare visitors" — the real sub-1% encounter mechanic.
//
// Normal encounter tables can't go below a 1% slot. Each row here gives a
// species its own exact chance, per encounter of that method on that map,
// on top of the normal table (it doesn't take a slot). The level comes from
// the slot the game rolled, so visitors match the area's levels.
//
// chance is in 1/10000ths: 50 = 0.5%, 100 = 1%, 200 = 2%.
// Ledger: docs/WALLY_RARE_VISITORS.md. Audit: tools/audit_wally_obtainability.py.

struct WallyRareVisitor
{
    u16 mapGroup:8;
    u16 mapNum:8;
    u8 area;            // WILD_AREA_LAND or WILD_AREA_WATER
    u16 species;
    u16 chance;         // out of 10000
};

#define VISITOR(map, areaType, sp, odds) {MAP_GROUP(map), MAP_NUM(map), areaType, sp, odds}

static const struct WallyRareVisitor sWallyRareVisitors[] =
{
    // Flower meadows of Route 117
    VISITOR(MAP_ROUTE117, WILD_AREA_LAND, SPECIES_FLABEBE_RED,       200), // 2%
    VISITOR(MAP_ROUTE117, WILD_AREA_LAND, SPECIES_COMFEY,             50), // 0.5%, famously hard to find
    VISITOR(MAP_ROUTE117, WILD_AREA_LAND, SPECIES_FARFETCHD_GALAR,    50), // 0.5%, far from home
    // Lilycove outskirts
    VISITOR(MAP_ROUTE121, WILD_AREA_LAND, SPECIES_MILCERY,            50), // 0.5%, drawn by sweet scents
    // Berry groves
    VISITOR(MAP_ROUTE123, WILD_AREA_LAND, SPECIES_POLTCHAGEIST_COUNTERFEIT, 100), // 1%
    // Boggy ground near Fallarbor
    VISITOR(MAP_ROUTE114, WILD_AREA_LAND, SPECIES_WOOPER_PALDEA,     200), // 2%
    // Placed here because their habitat's slots are all some species' only spot
    VISITOR(MAP_METEOR_FALLS_1F_2R, WILD_AREA_LAND, SPECIES_CLEFFA,            50), // 0.5%, meteor lore
    VISITOR(MAP_ROUTE114, WILD_AREA_LAND, SPECIES_DROWZEE,                    300), // 3%, hills near Fallarbor
    VISITOR(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM, WILD_AREA_LAND, SPECIES_EISCUE_ICE, 100), // 1%
    VISITOR(MAP_ROUTE120, WILD_AREA_LAND, SPECIES_FOONGUS,                    300), // 3%, forest floor
    VISITOR(MAP_ROUTE120, WILD_AREA_LAND, SPECIES_PANPOUR,                    200), // 2%, riverside
    VISITOR(MAP_NEW_MAUVILLE_INSIDE, WILD_AREA_LAND, SPECIES_KLEFKI,          200), // 2%, collects keys
    VISITOR(MAP_NEW_MAUVILLE_INSIDE, WILD_AREA_LAND, SPECIES_PORYGON,          50), // 0.5%, stray data
    VISITOR(MAP_PETALBURG_WOODS, WILD_AREA_LAND, SPECIES_MORELULL,            200), // 2%, glowing mushrooms
    VISITOR(MAP_ROUTE110, WILD_AREA_LAND, SPECIES_MORPEKO_FULL_BELLY,         200), // 2%, follows seeds
    VISITOR(MAP_ROUTE113, WILD_AREA_LAND, SPECIES_ZIGZAGOON_GALAR,             50), // 0.5%, Obstagoon line
    VISITOR(MAP_ROUTE121, WILD_AREA_LAND, SPECIES_ORICORIO_BAILE,             200), // 2%, nectar flowers
    VISITOR(MAP_SAFARI_ZONE_NORTHEAST, WILD_AREA_LAND, SPECIES_SCYTHER,       200), // 2%, Safari tall grass
};

#undef VISITOR
