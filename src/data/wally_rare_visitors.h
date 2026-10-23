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
    // Alternate forms (2026-10-17): regional variants, colors, patterns and sizes
    VISITOR(MAP_ROUTE110, WILD_AREA_LAND, SPECIES_RATTATA_ALOLA, 100), // 1%, stowaway from overseas freighters
    VISITOR(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM, WILD_AREA_LAND, SPECIES_SANDSHREW_ALOLA, 100), // 1%, ice-adapted
    VISITOR(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM, WILD_AREA_LAND, SPECIES_VULPIX_ALOLA, 50), // 0.5%, ice-adapted
    VISITOR(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM, WILD_AREA_LAND, SPECIES_DARUMAKA_GALAR, 50), // 0.5%, snow-country form
    VISITOR(MAP_JAGGED_PASS, WILD_AREA_LAND, SPECIES_DIGLETT_ALOLA, 100), // 1%, volcanic soil
    VISITOR(MAP_JAGGED_PASS, WILD_AREA_LAND, SPECIES_GEODUDE_ALOLA, 100), // 1%, magnetized rock
    VISITOR(MAP_ROUTE123, WILD_AREA_LAND, SPECIES_MEOWTH_ALOLA, 100), // 1%, pampered pets gone wild
    VISITOR(MAP_FIERY_PATH, WILD_AREA_LAND, SPECIES_GRIMER_ALOLA, 100), // 1%, eats the garbage others won't
    VISITOR(MAP_ROUTE119, WILD_AREA_LAND, SPECIES_PONYTA_GALAR, 50), // 0.5%, misty forest
    VISITOR(MAP_ROUTE120, WILD_AREA_LAND, SPECIES_SLOWPOKE_GALAR, 50), // 0.5%, Galarica bark nearby
    VISITOR(MAP_ROUTE120, WILD_AREA_LAND, SPECIES_STUNFISK_GALAR, 100), // 1%, mud flats
    VISITOR(MAP_ROUTE112, WILD_AREA_LAND, SPECIES_GROWLITHE_HISUI, 50), // 0.5%, ancient lineage
    VISITOR(MAP_NEW_MAUVILLE_INSIDE, WILD_AREA_LAND, SPECIES_VOLTORB_HISUI, 100), // 1%, old apricorn-wood stock
    VISITOR(MAP_MT_PYRE_EXTERIOR, WILD_AREA_LAND, SPECIES_ZORUA_HISUI, 50), // 0.5%, restless spirits
    VISITOR(MAP_SAFARI_ZONE_SOUTH, WILD_AREA_LAND, SPECIES_TAUROS_PALDEA_COMBAT, 100), // 1%, Safari herd
    VISITOR(MAP_SAFARI_ZONE_SOUTHWEST, WILD_AREA_LAND, SPECIES_TAUROS_PALDEA_BLAZE, 50), // 0.5%, Safari herd
    VISITOR(MAP_SAFARI_ZONE_NORTHEAST, WILD_AREA_LAND, SPECIES_TAUROS_PALDEA_AQUA, 50), // 0.5%, Safari herd
    VISITOR(MAP_ROUTE119, WILD_AREA_WATER, SPECIES_BASCULIN_BLUE_STRIPED, 200), // 2%, river school
    VISITOR(MAP_ROUTE124, WILD_AREA_WATER, SPECIES_SHELLOS_EAST, 200), // 2%, east sea
    VISITOR(MAP_ROUTE111, WILD_AREA_LAND, SPECIES_BURMY_SANDY, 200), // 2%, sand cloak
    VISITOR(MAP_NEW_MAUVILLE_INSIDE, WILD_AREA_LAND, SPECIES_BURMY_TRASH, 200), // 2%, trash cloak
    VISITOR(MAP_ROUTE104, WILD_AREA_LAND, SPECIES_FLABEBE_YELLOW, 200), // 2%, flower meadow
    VISITOR(MAP_ROUTE117, WILD_AREA_LAND, SPECIES_FLABEBE_ORANGE, 200), // 2%, flower meadow
    VISITOR(MAP_ROUTE119, WILD_AREA_LAND, SPECIES_FLABEBE_WHITE, 100), // 1%, wild flowers
    VISITOR(MAP_ROUTE123, WILD_AREA_LAND, SPECIES_FLABEBE_BLUE, 100), // 1%, berry gardens
    VISITOR(MAP_MT_PYRE_1F, WILD_AREA_LAND, SPECIES_PUMPKABOO_SMALL, 200), // 2%, pumpkin patch
    VISITOR(MAP_MT_PYRE_EXTERIOR, WILD_AREA_LAND, SPECIES_PUMPKABOO_LARGE, 200), // 2%, pumpkin patch
    VISITOR(MAP_MT_PYRE_EXTERIOR, WILD_AREA_LAND, SPECIES_PUMPKABOO_SUPER, 50), // 0.5%, pumpkin patch
    VISITOR(MAP_ROUTE103, WILD_AREA_LAND, SPECIES_SQUAWKABILLY_BLUE, 100), // 1%, flock
    VISITOR(MAP_ROUTE103, WILD_AREA_LAND, SPECIES_SQUAWKABILLY_YELLOW, 100), // 1%, flock
    VISITOR(MAP_ROUTE103, WILD_AREA_LAND, SPECIES_SQUAWKABILLY_WHITE, 50), // 0.5%, flock
    VISITOR(MAP_ROUTE128, WILD_AREA_WATER, SPECIES_TATSUGIRI_DROOPY, 200), // 2%, reef
    VISITOR(MAP_ROUTE128, WILD_AREA_WATER, SPECIES_TATSUGIRI_STRETCHY, 200), // 2%, reef
    VISITOR(MAP_MT_PYRE_2F, WILD_AREA_LAND, SPECIES_SINISTEA_ANTIQUE, 20), // 0.2%, the genuine article is rare
    VISITOR(MAP_MT_PYRE_2F, WILD_AREA_LAND, SPECIES_POLTCHAGEIST_ARTISAN, 20), // 0.2%, the genuine article is rare
    VISITOR(MAP_ROUTE123, WILD_AREA_LAND, SPECIES_INDEEDEE_F, 200), // 2%, female form
    VISITOR(MAP_MT_PYRE_4F, WILD_AREA_LAND, SPECIES_GIMMIGHOUL_CHEST, 50), // 0.5%, guards a hidden chest
    // Vivillon wing patterns: each region of Hoenn hosts different migrants
    VISITOR(MAP_ROUTE119, WILD_AREA_LAND, SPECIES_VIVILLON_JUNGLE, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE119, WILD_AREA_LAND, SPECIES_VIVILLON_MONSOON, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE120, WILD_AREA_LAND, SPECIES_VIVILLON_RIVER, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE120, WILD_AREA_LAND, SPECIES_VIVILLON_GARDEN, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE111, WILD_AREA_LAND, SPECIES_VIVILLON_SANDSTORM, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE111, WILD_AREA_LAND, SPECIES_VIVILLON_SUN, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE113, WILD_AREA_LAND, SPECIES_VIVILLON_HIGH_PLAINS, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE114, WILD_AREA_LAND, SPECIES_VIVILLON_MEADOW, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE114, WILD_AREA_LAND, SPECIES_VIVILLON_CONTINENTAL, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE115, WILD_AREA_LAND, SPECIES_VIVILLON_ARCHIPELAGO, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE115, WILD_AREA_LAND, SPECIES_VIVILLON_OCEAN, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE121, WILD_AREA_LAND, SPECIES_VIVILLON_ELEGANT, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE121, WILD_AREA_LAND, SPECIES_VIVILLON_MODERN, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE123, WILD_AREA_LAND, SPECIES_VIVILLON_SAVANNA, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE123, WILD_AREA_LAND, SPECIES_VIVILLON_FANCY, 50), // 0.5%, migrating
    VISITOR(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM, WILD_AREA_LAND, SPECIES_VIVILLON_POLAR, 50), // 0.5%, migrating
    VISITOR(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM, WILD_AREA_LAND, SPECIES_VIVILLON_TUNDRA, 50), // 0.5%, migrating
    VISITOR(MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM, WILD_AREA_LAND, SPECIES_VIVILLON_ICY_SNOW, 50), // 0.5%, migrating
    VISITOR(MAP_ROUTE118, WILD_AREA_LAND, SPECIES_VIVILLON_MARINE, 50), // 0.5%, migrating
    VISITOR(MAP_SAFARI_ZONE_NORTHWEST, WILD_AREA_LAND, SPECIES_VIVILLON_POKEBALL, 50), // 0.5%, migrating
    // Spawn audit 2026-10-23: starters, pseudo-legendaries and historically scarce species
    // moved here (habitat-fitting map, exact chance, no table slot).
    VISITOR(MAP_FIERY_PATH, WILD_AREA_LAND, SPECIES_TEPIG, 100), // 1%, fire pig that roasts berries with its nose: volcanic cave
    VISITOR(MAP_ROUTE111, WILD_AREA_LAND, SPECIES_FUECOCO, 100), // 1%, fire croc that basks on warm ground: desert
    VISITOR(MAP_ROUTE112, WILD_AREA_LAND, SPECIES_CHIMCHAR, 100), // 1%, Pokédex: lives on rugged mountains; Route 112 mountain trail
    VISITOR(MAP_ROUTE114, WILD_AREA_LAND, SPECIES_LITTEN, 100), // 1%, solitary fire cat of open country, not a graveyard tower
    VISITOR(MAP_ROUTE111, WILD_AREA_LAND, SPECIES_FENNEKIN, 100), // 1%, a fennec fox: desert
    VISITOR(MAP_ROUTE117, WILD_AREA_LAND, SPECIES_SCORBUNNY, 100), // 1%, field rabbit that loves to run: grassy route
    VISITOR(MAP_ROUTE116, WILD_AREA_LAND, SPECIES_GROOKEY, 100), // 1%, forest chimp with a stick: wooded route
    VISITOR(MAP_ROUTE102, WILD_AREA_LAND, SPECIES_SPRIGATITO, 100), // 1%, grass kitten of meadows, near towns
    VISITOR(MAP_ROUTE114, WILD_AREA_WATER, SPECIES_TOTODILE, 100), // 1%, river croc: Route 114 river
    VISITOR(MAP_ROUTE120, WILD_AREA_WATER, SPECIES_FROAKIE, 100), // 1%, pond frog: Route 120 ponds and marsh
    VISITOR(MAP_ROUTE119, WILD_AREA_WATER, SPECIES_SOBBLE, 100), // 1%, shy river lizard: Route 119 river
    VISITOR(MAP_ROUTE117, WILD_AREA_WATER, SPECIES_QUAXLY, 100), // 1%, duckling of clean ponds: Route 117 pond
    VISITOR(MAP_ROUTE118, WILD_AREA_WATER, SPECIES_SQUIRTLE, 100), // 1%, turtle of shores and waterways
    VISITOR(MAP_ROUTE105, WILD_AREA_WATER, SPECIES_POPPLIO, 100), // 1%, sea lion of the open coast
    VISITOR(MAP_ROUTE109, WILD_AREA_WATER, SPECIES_OSHAWOTT, 100), // 1%, otter of beaches and shallows
    VISITOR(MAP_ROUTE128, WILD_AREA_WATER, SPECIES_PIPLUP, 100), // 1%, penguin of cold open sea
    VISITOR(MAP_METEOR_FALLS_STEVENS_CAVE, WILD_AREA_LAND, SPECIES_METANG, 100), // 1%, pseudo-legendary line stays a 1% find
    VISITOR(MAP_SKY_PILLAR_1F, WILD_AREA_LAND, SPECIES_DRAGONAIR, 100), // 1%, pseudo-legendary line stays a 1% find
    VISITOR(MAP_VICTORY_ROAD_B2F, WILD_AREA_LAND, SPECIES_HAKAMO_O, 100), // 1%, pseudo-legendary line stays a 1% find
    VISITOR(MAP_ROUTE104, WILD_AREA_LAND, SPECIES_HAPPINY, 100), // 1%, baby, egg/rare in every game
    VISITOR(MAP_ROUTE113, WILD_AREA_LAND, SPECIES_MAGBY, 100), // 1%, baby, egg/rare
    VISITOR(MAP_ROUTE116, WILD_AREA_LAND, SPECIES_AUDINO, 100), // 1%, only in rustling grass in Unova
    VISITOR(MAP_GRANITE_CAVE_B1F, WILD_AREA_LAND, SPECIES_DUNSPARCE, 100), // 1%, famously rare (Dark Cave)
    VISITOR(MAP_METEOR_FALLS_B1F_2R, WILD_AREA_LAND, SPECIES_ARCHEN, 100), // 1%, fossil Pokémon
    VISITOR(MAP_ROUTE130, WILD_AREA_WATER, SPECIES_LAPRAS, 100), // 1%, gift or once-a-week in every game
    VISITOR(MAP_NEW_MAUVILLE_ENTRANCE, WILD_AREA_LAND, SPECIES_ELEKID, 100), // 1%, baby, egg/rare
    VISITOR(MAP_VICTORY_ROAD_B1F, WILD_AREA_LAND, SPECIES_TYROGUE, 100), // 1%, baby, gift/egg
    VISITOR(MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM, WILD_AREA_LAND, SPECIES_SMOOCHUM, 100), // 1%, baby, egg/rare
    VISITOR(MAP_ROUTE111, WILD_AREA_LAND, SPECIES_ORTHWORM, 400), // 4%, desert worm (Asado Desert)
    VISITOR(MAP_ROUTE111, WILD_AREA_LAND, SPECIES_CAPSAKID, 400), // 4%, chili of dry, sunny land
    VISITOR(MAP_JAGGED_PASS, WILD_AREA_LAND, SPECIES_TINKATINK, 100), // 1%, forges hammers from scrap metal in mountains
    VISITOR(MAP_ROUTE123, WILD_AREA_LAND, SPECIES_MIME_JR, 100), // 1%, mimic baby of gardens (Trophy Garden); scarce
    VISITOR(MAP_RUSTURF_TUNNEL, WILD_AREA_LAND, SPECIES_ROGGENROLA, 100), // 1%, cave rock
    VISITOR(MAP_ROUTE110, WILD_AREA_LAND, SPECIES_TRUBBISH, 500), // 5%, garbage bag Pokémon near cities
    VISITOR(MAP_ROUTE114, WILD_AREA_LAND, SPECIES_LITLEO, 400), // 4%, lion cub of grassland
    VISITOR(MAP_ROUTE120, WILD_AREA_LAND, SPECIES_FLAMIGO, 100), // 1%, flamingo of wetlands
    VISITOR(MAP_SKY_PILLAR_3F, WILD_AREA_LAND, SPECIES_SOLOSIS, 100), // 1%, cell-like Pokémon said to come from space
    VISITOR(MAP_ROUTE119, WILD_AREA_WATER, SPECIES_BASCULIN, 100), // 1%, river fish; joins the Blue-Striped form on Route 119
    VISITOR(MAP_ROUTE121, WILD_AREA_LAND, SPECIES_ZORUA, 100), // 1%, illusion fox of forests (Lostlorn Forest); scarce
};

#undef VISITOR
