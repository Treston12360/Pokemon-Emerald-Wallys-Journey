static const u16 sFrontierExchangeCorner_Decor1[] =
{
    DECOR_KISS_POSTER,
    DECOR_KISS_CUSHION,
    DECOR_SMOOCHUM_DOLL,
    DECOR_TOGEPI_DOLL,
    DECOR_MEOWTH_DOLL,
    DECOR_CLEFAIRY_DOLL,
    DECOR_DITTO_DOLL,
    DECOR_CYNDAQUIL_DOLL,
    DECOR_CHIKORITA_DOLL,
    DECOR_TOTODILE_DOLL,
    ITEM_LIST_END
};

static const u16 sFrontierExchangeCorner_Decor2[] =
{
    DECOR_LAPRAS_DOLL,
    DECOR_SNORLAX_DOLL,
    DECOR_VENUSAUR_DOLL,
    DECOR_CHARIZARD_DOLL,
    DECOR_BLASTOISE_DOLL,
    ITEM_LIST_END
};

// Wally's Journey: vitamins do nothing with Stat Points, so this clerk sells training goods instead
static const enum Item sFrontierExchangeCorner_Vitamins[] =
{
    ITEM_RARE_CANDY,
    ITEM_PP_UP,
    ITEM_PP_MAX,
    ITEM_BOTTLE_CAP,
    ITEM_GOLD_BOTTLE_CAP,
    ITEM_ABILITY_PATCH,
    ITEM_EVIOLITE,
    ITEM_WEAKNESS_POLICY,
    ITEM_EXPERT_BELT,
    ITEM_LIGHT_CLAY,
    ITEM_HEAVY_DUTY_BOOTS,
    ITEM_AIR_BALLOON,
    ITEM_LIST_END
};

static const enum Item sFrontierExchangeCorner_HoldItems[] =
{
    ITEM_LEFTOVERS,
    ITEM_WHITE_HERB,
    ITEM_QUICK_CLAW,
    ITEM_MENTAL_HERB,
    ITEM_BRIGHT_POWDER,
    ITEM_CHOICE_BAND,
    ITEM_KINGS_ROCK,
    ITEM_FOCUS_BAND,
    ITEM_SCOPE_LENS,
    ITEM_LIFE_ORB,
    ITEM_CHOICE_SPECS,
    ITEM_CHOICE_SCARF,
    ITEM_FOCUS_SASH,
    ITEM_ASSAULT_VEST,
    ITEM_ROCKY_HELMET,
    ITEM_LIST_END
};

static const u8 *const sFrontierExchangeCorner_Decor1Descriptions[] =
{
    BattleFrontier_ExchangeServiceCorner_Text_KissPosterDesc,
    BattleFrontier_ExchangeServiceCorner_Text_KissCushionDesc,
    BattleFrontier_ExchangeServiceCorner_Text_SmoochumDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_TogepiDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_MeowthDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_ClefairyDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_DittoDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_CyndaquilDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_ChikoritaDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_TotodileDollDesc,
    gText_Exit,
};

static const u8 *const sFrontierExchangeCorner_Decor2Descriptions[] =
{
    BattleFrontier_ExchangeServiceCorner_Text_LargeDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_LargeDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_LargeDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_LargeDollDesc,
    BattleFrontier_ExchangeServiceCorner_Text_LargeDollDesc,
    gText_Exit
};

static const u8 *const sFrontierExchangeCorner_VitaminsDescriptions[] =
{
    COMPOUND_STRING("Raises the level of one Pokémon\nby one."),
    COMPOUND_STRING("Raises the max PP of one move\na little."),
    COMPOUND_STRING("Raises the max PP of one move\nas far as it goes."),
    COMPOUND_STRING("Hyper Training: maxes one of a\nPokémon's IVs."),
    COMPOUND_STRING("Hyper Training: maxes all of a\nPokémon's IVs."),
    COMPOUND_STRING("Switches a Pokémon to any of\nits other Abilities."),
    COMPOUND_STRING("Raises Defense and Sp. Def of a\nPokémon that can still evolve."),
    COMPOUND_STRING("Sharply raises Attack and Sp. Atk\nwhen hit super effectively."),
    COMPOUND_STRING("Powers up super effective\nmoves a little."),
    COMPOUND_STRING("Makes Reflect, Light Screen and\nAurora Veil last longer."),
    COMPOUND_STRING("Protects the holder from\nentry hazards."),
    COMPOUND_STRING("The holder floats over Ground\nmoves until it is hit."),
    gText_Exit
};

static const u8 *const sFrontierExchangeCorner_HoldItemsDescriptions[] =
{
    BattleFrontier_ExchangeServiceCorner_Text_LeftoversDesc,
    BattleFrontier_ExchangeServiceCorner_Text_WhiteHerbDesc,
    BattleFrontier_ExchangeServiceCorner_Text_QuickClawDesc,
    BattleFrontier_ExchangeServiceCorner_Text_MentalHerbDesc,
    BattleFrontier_ExchangeServiceCorner_Text_BrightpowderDesc,
    BattleFrontier_ExchangeServiceCorner_Text_ChoiceBandDesc,
    BattleFrontier_ExchangeServiceCorner_Text_KingsRockDesc,
    BattleFrontier_ExchangeServiceCorner_Text_FocusBandDesc,
    BattleFrontier_ExchangeServiceCorner_Text_ScopeLensDesc,
    COMPOUND_STRING("Powers up moves, but the holder\nloses a little HP with each hit."),
    COMPOUND_STRING("Boosts Sp. Atk, but only one\nmove can be used."),
    COMPOUND_STRING("Boosts Speed, but only one\nmove can be used."),
    COMPOUND_STRING("Survives one hit that would KO\nthe holder from full HP."),
    COMPOUND_STRING("Raises Sp. Def, but the holder\ncan only use attacking moves."),
    COMPOUND_STRING("Hurts attackers that make\ncontact with the holder."),
    gText_Exit
};
