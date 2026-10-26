# Wally's Journey full Hoenn wild redesign (2026-10-23).
# Each table: (minLv, maxLv, {SPECIES: weight%}). Weights are turned into slots by the builder.
# Visitors: {SPECIES: percent} rolled on top of the table (no slot). Rarities <=1% live here.
# Global slot rates after this pass:
#   land 20/20/10/10/10/10/5/5/4/4/1/1, surf 35/30/20/10/5, rock smash 40/30/15/10/5,
#   old rod 70/30, good rod 60/20/20, super rod 40/40/15/4/1.

D = {}
def area(name, **kw): D[name] = kw

# ---------------------------------------------------------------- shared water templates
def sea(a, b, c, d, e, lo=25, hi=35):
    return (lo, hi, {a: 35, b: 30, c: 20, d: 10, e: 5})
OLD_SEA   = (5, 10, {'MAGIKARP': 70, 'TENTACOOL': 30})
OLD_FRESH = (5, 10, {'MAGIKARP': 70, 'GOLDEEN': 30})

# ================================================================= ROUTES
area('ROUTE101', land=(2, 3, {'ZIGZAGOON': 30, 'WURMPLE': 20, 'POOCHYENA': 20, 'BIDOOF': 10, 'LILLIPUP': 5,
                              'PATRAT': 5, 'ROOKIDEE': 4, 'SKWOVET': 4, 'LECHONK': 1, 'PIDGEY': 1}))

area('ROUTE102', land=(3, 4, {'WURMPLE': 20, 'ZIGZAGOON': 20, 'LOTAD': 10, 'POOCHYENA': 10, 'PIDGEY': 10, 'SEEDOT': 10,
                              'NYMBLE': 5, 'TAROUNTULA': 5, 'RALTS': 4, 'BUNNELBY': 4, 'DEERLING_SPRING': 1, 'SKWOVET': 1}),
     v_land={'SPRIGATITO': 1, 'MUDKIP': 1},
     water=(20, 30, {'LOTAD': 35, 'MARILL': 30, 'SURSKIT': 20, 'GOLDEEN': 10, 'PSYDUCK': 5}),
     old=OLD_FRESH, good=(10, 25, {'GOLDEEN': 60, 'MAGIKARP': 20, 'CORPHISH': 20}),
     super=(20, 35, {'CORPHISH': 40, 'GOLDEEN': 40, 'BARBOACH': 15, 'SEAKING': 5}))

area('ROUTE103', land=(2, 4, {'ZIGZAGOON': 20, 'POOCHYENA': 20, 'WINGULL': 20, 'FLETCHLING': 10, 'PIDOVE': 10,
                              'SQUAWKABILLY_GREEN': 5, 'WINGULL ': 5, 'WATTREL': 4, 'RATTATA': 4, 'SKWOVET': 1, 'ZIGZAGOON ': 1}),
     v_land={'SQUAWKABILLY_BLUE': 1, 'SQUAWKABILLY_YELLOW': 1, 'SQUAWKABILLY_WHITE': 0.5},
     water=sea('TENTACOOL', 'WINGULL', 'FINNEON', 'PELIPPER', 'BUIZEL'),
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'FINNEON': 20, 'WAILMER': 20}),
     super=(25, 40, {'WAILMER': 40, 'CARVANHA': 40, 'SHARPEDO': 15, 'LUMINEON': 5}))

area('ROUTE104', land=(4, 5, {'ZIGZAGOON': 20, 'WURMPLE': 20, 'TAILLOW': 15, 'MARILL': 10, 'WINGULL': 10, 'PIKIPEK': 10,
                              'GOSSIFLEUR': 5, 'PANSAGE': 4, 'POOCHYENA': 4, 'STARLY': 1, 'CHIKORITA': 1}),
     v_land={'FLABEBE_YELLOW': 2, 'HAPPINY': 1},
     water=sea('WINGULL', 'TENTACOOL', 'WISHIWASHI_SOLO', 'PELIPPER', 'FINNEON'),
     old=(5, 10, {'MAGIKARP': 70, 'WISHIWASHI_SOLO': 30}), good=(10, 25, {'MAGIKARP': 60, 'WISHIWASHI_SOLO': 20, 'KRABBY': 20}),
     super=(20, 40, {'WAILMER': 40, 'MAGIKARP': 40, 'KINGLER': 15, 'GYARADOS': 5}))

area('ROUTE105', water=sea('TENTACOOL', 'FINNEON', 'WINGULL', 'PELIPPER', 'MANTYKE'),
     v_water={'POPPLIO': 1},
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'FINNEON': 20, 'WAILMER': 20}),
     super=(25, 40, {'WAILMER': 40, 'LUMINEON': 40, 'SHARPEDO': 15, 'TENTACRUEL': 5}),
     )

area('ROUTE106', water=sea('TENTACOOL', 'SHELLOS', 'WINGULL', 'PELIPPER', 'KRABBY'),
     v_water={'FRILLISH': 1},
     old=OLD_SEA, good=(10, 25, {'KRABBY': 60, 'TENTACOOL': 20, 'WAILMER': 20}),
     super=(25, 40, {'WAILMER': 40, 'KINGLER': 40, 'SHARPEDO': 15, 'GASTRODON': 5}),
     )

area('ROUTE107', water=sea('TENTACOOL', 'BUIZEL', 'WINGULL', 'PELIPPER', 'FLOATZEL'),
     v_water={'WIMPOD': 1},
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'WAILMER': 20, 'CHINCHOU': 20}),
     super=(25, 40, {'WAILMER': 40, 'CHINCHOU': 40, 'SHARPEDO': 15, 'LANTURN': 5}))

area('ROUTE108', water=sea('TENTACOOL', 'WINGULL', 'ALOMOMOLA', 'PELIPPER', 'MANTYKE'),
     v_water={'DHELMISE': 1},
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'WAILMER': 20, 'ALOMOMOLA': 20}),
     super=(25, 40, {'WAILMER': 40, 'SHARPEDO': 40, 'ALOMOMOLA': 15, 'TENTACRUEL': 5}),
     v_super={'DREDNAW': 1})

area('ROUTE109', water=sea('WINGULL', 'TENTACOOL', 'SHELLOS_WEST', 'PELIPPER', 'WIGLETT'),
     v_water={'OSHAWOTT': 1},
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'WAILMER': 20, 'STARYU': 20}),
     super=(25, 40, {'WAILMER': 40, 'STARYU': 40, 'SHARPEDO': 15, 'LANTURN': 5}))

area('ROUTE110', land=(12, 13, {'ELECTRIKE': 20, 'GULPIN': 20, 'PLUSLE': 10, 'MINUN': 10, 'ODDISH': 10, 'ZIGZAGOON': 10,
                                'POOCHYENA': 5, 'BLITZLE': 5, 'MEOWTH': 4, 'PICHU': 4, 'MORPEKO': 1, 'PIKACHU': 1}),
     v_land={'TADBULB': 1, 'TRUBBISH': 5, 'MEOWTH_ALOLA': 1, 'GLAMEOW': 2, 'GRIMER_ALOLA': 1, 'RATTATA_ALOLA': 1, 'MORPEKO_FULL_BELLY': 2},
     water=sea('TENTACOOL', 'WINGULL', 'FINNEON', 'PELIPPER', 'WAILMER'),
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'WAILMER': 20, 'MAGIKARP': 20}),
     super=(25, 40, {'WAILMER': 40, 'CARVANHA': 40, 'SHARPEDO': 15, 'TENTACRUEL': 5}))

area('ROUTE111', land=(19, 22, {'SANDSHREW': 20, 'TRAPINCH': 20, 'CACNEA': 15, 'BALTOY': 10, 'SANDILE': 10, 'SKORUPI': 5,
                                'HELIOPTILE': 5, 'SILICOBRA': 5, 'HIPPOPOTAS': 4, 'MARACTUS': 4, 'CYCLIZAR': 1, 'KLAWF': 1}),
     v_land={'ORTHWORM': 4, 'CAPSAKID': 4, 'BURMY_SANDY': 2, 'VIVILLON_SANDSTORM': 0.5, 'VIVILLON_SUN': 0.5,
             'FUECOCO': 1, 'FENNEKIN': 1, 'DARUMAKA': 3},
     water=(20, 30, {'MARILL': 35, 'GOLDEEN': 30, 'BARBOACH': 20, 'PSYDUCK': 10, 'AZUMARILL': 5}),
     rock=(10, 20, {'GEODUDE': 40, 'NACLI': 30, 'DWEBBLE': 15, 'ROGGENROLA': 10, 'GRAVELER': 5}),
     v_rock={'ANORITH': 1, 'STONJOURNER': 1, 'LARVITAR': 1},
     old=OLD_FRESH, good=(10, 25, {'GOLDEEN': 60, 'BARBOACH': 20, 'MAGIKARP': 20}),
     super=(20, 35, {'BARBOACH': 40, 'GOLDEEN': 40, 'WHISCASH': 15, 'SEAKING': 5}))

area('ROUTE112', land=(14, 16, {'NUMEL': 30, 'PONYTA': 15, 'GEODUDE': 15, 'MACHOP': 10, 'ROLYCOLY': 10, 'SKIDDO': 10,
                                'SLUGMA': 5, 'SALANDIT': 4, 'CHARMANDER': 1}),
     v_land={'CHIMCHAR': 1, 'GROWLITHE_HISUI': 0.5})

area('ROUTE113', land=(14, 16, {'SPINDA': 25, 'SLUGMA': 20, 'SANDSHREW': 15, 'SPEAROW': 15, 'NUMEL': 10, 'SKARMORY': 5,
                                'CHARCADET': 4, 'TORCHIC': 1}),
     v_land={'ZIGZAGOON_GALAR': 0.5, 'VIVILLON_HIGH_PLAINS': 0.5, 'MAGBY': 1})

area('ROUTE114', land=(15, 18, {'SWABLU': 20, 'LOTAD': 15, 'SEVIPER': 10, 'ZANGOOSE': 10, 'LOMBRE': 10, 'NUZLEAF': 10,
                                'MUNNA': 5, 'HOPPIP': 5, 'MASCHIFF': 4, 'FALINKS': 4, 'SEEDOT': 1, 'STUNKY': 1}),
     v_land={'WOOPER_PALDEA': 2, 'DROWZEE': 3, 'LITLEO': 4, 'VIVILLON_MEADOW': 0.5, 'VIVILLON_CONTINENTAL': 0.5, 'LITTEN': 1},
     water=(20, 30, {'LOTAD': 35, 'MARILL': 30, 'GOLDEEN': 20, 'LOMBRE': 10, 'WOOPER': 5}),
     v_water={'TOTODILE': 1},
     rock=(10, 20, {'GEODUDE': 40, 'ROGGENROLA': 30, 'NOSEPASS': 15, 'BONSLY': 10, 'ROLYCOLY': 5}),
     v_rock={'ANORITH': 1},
     old=OLD_FRESH, good=(10, 25, {'GOLDEEN': 60, 'BARBOACH': 20, 'MAGIKARP': 20}),
     super=(20, 35, {'BARBOACH': 40, 'GOLDEEN': 40, 'WHISCASH': 15, 'SEAKING': 5}))

area('ROUTE115', land=(23, 26, {'TAILLOW': 20, 'SWABLU': 20, 'JIGGLYPUFF': 10, 'WINGULL': 10, 'SWELLOW': 10, 'CRABRAWLER': 10,
                                'BUNEARY': 5, 'MANKEY': 5, 'ROCKRUFF': 4}),
     v_land={'VIVILLON_ARCHIPELAGO': 0.5, 'VIVILLON_OCEAN': 0.5},
     water=sea('TENTACOOL', 'WINGULL', 'STARYU', 'PELIPPER', 'SHELLOS'),
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'KRABBY': 20, 'WAILMER': 20}),
     super=(25, 40, {'WAILMER': 40, 'SHARPEDO': 40, 'KINGLER': 15, 'STARMIE': 5}))

area('ROUTE116', land=(6, 8, {'ZIGZAGOON': 15, 'TAILLOW': 15, 'WHISMUR': 15, 'NINCADA': 15, 'POOCHYENA': 10, 'SKITTY': 5,
                              'EKANS': 5, 'SENTRET': 5, 'KRICKETOT': 4, 'PURRLOIN': 1, 'ABRA': 1}),
     v_land={'GROOKEY': 1, 'AUDINO': 1})

area('ROUTE117', land=(13, 14, {'ODDISH': 20, 'ROSELIA': 15, 'POOCHYENA': 10, 'ILLUMISE': 10, 'VOLBEAT': 10, 'SEEDOT': 10,
                                'FIDOUGH': 5, 'TANDEMAUS': 5, 'MINCCINO': 4, 'YAMPER': 4, 'BUDEW': 1, 'MARILL': 1}),
     v_land={'SPRITZEE': 2, 'FLABEBE_RED': 2, 'FLABEBE_ORANGE': 2, 'COMFEY': 0.5, 'FARFETCHD_GALAR': 0.5, 'SCORBUNNY': 1},
     water=(20, 30, {'MARILL': 35, 'SURSKIT': 30, 'GOLDEEN': 20, 'TYMPOLE': 10, 'MASQUERAIN': 5}),
     v_water={'QUAXLY': 1},
     old=OLD_FRESH, good=(10, 25, {'GOLDEEN': 60, 'POLIWAG': 20, 'CORPHISH': 20}),
     super=(20, 35, {'CORPHISH': 40, 'POLIWAG': 40, 'CRAWDAUNT': 15, 'POLIWHIRL': 5}))

area('ROUTE118', land=(24, 27, {'ELECTRIKE': 15, 'LINOONE': 15, 'ZIGZAGOON': 10, 'MANECTRIC': 10, 'WINGULL': 10, 'BLITZLE': 10,
                                'SKIDDO': 5, 'TOGEDEMARU': 5, 'SHINX': 4, 'PELIPPER': 4, 'KECLEON': 1}),
     v_land={'VIVILLON_MARINE': 0.5},
     water=sea('TENTACOOL', 'WINGULL', 'ARROKUDA', 'PELIPPER', 'CARVANHA'),
     v_water={'SQUIRTLE': 1},
     old=OLD_SEA, good=(10, 25, {'CARVANHA': 60, 'TENTACOOL': 20, 'ARROKUDA': 20}),
     super=(25, 40, {'CARVANHA': 40, 'SHARPEDO': 40, 'BARRASKEWDA': 15, 'WAILMER': 5}))

area('ROUTE119', land=(24, 27, {'ODDISH': 15, 'LINOONE': 15, 'BELLSPROUT': 10, 'CHATOT': 10, 'CUTIEFLY': 10, 'TROPIUS': 10,
                                'VENONAT': 5, 'KARRABLAST': 5, 'FOMANTIS': 4, 'PARAS': 4, 'KECLEON': 1, 'GOOMY': 1}),
     v_land={'TOEDSCOOL': 1, 'HAWLUCHA': 2, 'FLABEBE_WHITE': 1, 'VIVILLON_JUNGLE': 0.5, 'VIVILLON_MONSOON': 0.5, 'ZIGZAGOON': 5},
     water=(25, 33, {'MARILL': 35, 'GOLDEEN': 30, 'BASCULIN_BLUE_STRIPED': 20, 'CHEWTLE': 10, 'BASCULIN': 5}),
     v_water={'SOBBLE': 1},
     old=OLD_FRESH, good=(10, 25, {'BASCULIN_RED_STRIPED': 60, 'GOLDEEN': 20, 'CARVANHA': 20}),
     super=(25, 40, {'CARVANHA': 40, 'BASCULIN_RED_STRIPED': 40, 'BASCULIN_WHITE_STRIPED': 15, 'SHARPEDO': 5}))

area('ROUTE120', land=(25, 27, {'MIGHTYENA': 15, 'ODDISH': 15, 'POOCHYENA': 10, 'SURSKIT': 10, 'YANMA': 10, 'TANGELA': 10,
                                'SHELMET': 5, 'PACHIRISU': 5, 'CARNIVINE': 4, 'ABSOL': 4, 'ROWLET': 1, 'KECLEON': 1}),
     v_land={'FOONGUS': 3, 'PANPOUR': 2, 'GRUBBIN': 3, 'EMOLGA': 2, 'INDEEDEE_M': 1, 'INDEEDEE_F': 1, 'IMPIDIMP': 1, 'HATENNA': 1, 'PONYTA_GALAR': 0.5, 'SLOWPOKE_GALAR': 0.5, 'STUNFISK_GALAR': 1,
             'VIVILLON_RIVER': 0.5, 'VIVILLON_GARDEN': 0.5, 'FLAMIGO': 1},
     water=(25, 33, {'MARILL': 35, 'SURSKIT': 30, 'WOOPER': 20, 'DEWPIDER': 10, 'QUAGSIRE': 5}),
     v_water={'FROAKIE': 1},
     old=OLD_FRESH, good=(10, 25, {'POLIWAG': 60, 'BARBOACH': 20, 'GOLDEEN': 20}),
     super=(25, 40, {'BARBOACH': 40, 'POLIWHIRL': 40, 'WHISCASH': 15, 'SEAKING': 5}))

area('ROUTE121', land=(26, 28, {'ODDISH': 15, 'MIGHTYENA': 15, 'POOCHYENA': 10, 'SHUPPET': 10, 'GLOOM': 10, 'WINGULL': 10,
                                'NICKIT': 5, 'COTTONEE': 5, 'SWIRLIX': 4, 'SHROODLE': 4, 'EEVEE': 1, 'KECLEON': 1}),
     v_land={'MILCERY': 0.5, 'ORICORIO_BAILE': 2, 'VIVILLON_ELEGANT': 0.5, 'VIVILLON_MODERN': 0.5, 'ZORUA': 1,
             'ESPURR': 2, 'GOTHITA': 2},
     water=sea('TENTACOOL', 'WINGULL', 'CRAMORANT', 'PELIPPER', 'STARYU'),
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'WAILMER': 20, 'STARYU': 20}),
     super=(25, 40, {'WAILMER': 40, 'STARYU': 40, 'SHARPEDO': 15, 'STARMIE': 5}))

area('ROUTE122', water=sea('TENTACOOL', 'WINGULL', 'FRILLISH', 'PELIPPER', 'TENTACRUEL'),
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'WAILMER': 20, 'FRILLISH': 20}),
     super=(25, 40, {'WAILMER': 40, 'SHARPEDO': 40, 'FRILLISH': 15, 'JELLICENT': 5}))

area('ROUTE123', land=(26, 28, {'ODDISH': 15, 'MIGHTYENA': 15, 'CHERUBI': 10, 'POOCHYENA': 10, 'SHUPPET': 10, 'GLOOM': 10,
                                'SMOLIV': 5, 'EXEGGCUTE': 5, 'FLABEBE': 5, 'PETILIL': 4, 'MUNCHLAX': 1, 'KECLEON': 1}),
     v_land={'POLTCHAGEIST_COUNTERFEIT': 1, 'FLABEBE_BLUE': 1,
             'VIVILLON_SAVANNA': 0.5, 'VIVILLON_FANCY': 0.5, 'MIME_JR': 1},
     water=sea('TENTACOOL', 'WINGULL', 'FINNEON', 'PELIPPER', 'TENTACRUEL'),
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'WAILMER': 20, 'FINNEON': 20}),
     super=(25, 40, {'WAILMER': 40, 'SHARPEDO': 40, 'LUMINEON': 15, 'WAILORD': 5}))

LATE = dict(lo=30, hi=40)
area('ROUTE124', water=sea('TENTACOOL', 'WINGULL', 'CLAUNCHER', 'PELIPPER', 'FINIZEN', **LATE),
     v_water={'SHELLOS_EAST': 2},
     old=OLD_SEA, good=(15, 30, {'TENTACOOL': 60, 'CLAUNCHER': 20, 'WAILMER': 20}),
     super=(30, 45, {'WAILMER': 40, 'SHARPEDO': 40, 'CLAUNCHER': 15, 'CLAWITZER': 5}),
     v_super={'CARRACOSTA': 1})
area('ROUTE125', water=sea('TENTACOOL', 'QWILFISH', 'WINGULL', 'PELIPPER', 'SPHEAL', **LATE),
     v_water={'PIPLUP': 1, 'MAREANIE': 1},
     old=OLD_SEA, good=(15, 30, {'TENTACOOL': 60, 'QWILFISH': 20, 'WAILMER': 20}),
     super=(30, 45, {'WAILMER': 40, 'SHARPEDO': 40, 'QWILFISH': 15, 'TENTACRUEL': 5}),
     v_super={'SEALEO': 1})
area('ROUTE126', water=sea('TENTACOOL', 'WINGULL', 'SKRELP', 'PELIPPER', 'CHINCHOU', **LATE),
     old=OLD_SEA, good=(15, 30, {'TENTACOOL': 60, 'SKRELP': 20, 'CHINCHOU': 20}),
     super=(30, 45, {'WAILMER': 40, 'SHARPEDO': 40, 'LANTURN': 15, 'DRAGALGE': 5}),
     v_super={'MILOTIC': 1})
area('ROUTE127', water=sea('TENTACOOL', 'WINGULL', 'REMORAID', 'PELIPPER', 'CHINCHOU', **LATE),
     v_water={'MAREANIE': 1},
     old=OLD_SEA, good=(15, 30, {'REMORAID': 60, 'TENTACOOL': 20, 'WAILMER': 20}),
     super=(30, 45, {'WAILMER': 40, 'SHARPEDO': 40, 'OCTILLERY': 15, 'WUGTRIO': 5}))
area('ROUTE128', water=sea('TENTACOOL', 'LUVDISC', 'WINGULL', 'PELIPPER', 'GASTRODON', **LATE),
     v_water={'TATSUGIRI_DROOPY': 2, 'TATSUGIRI_STRETCHY': 2, 'BRUXISH': 2},
     old=OLD_SEA, good=(15, 30, {'LUVDISC': 60, 'TENTACOOL': 20, 'WAILMER': 20}),
     super=(30, 45, {'LUVDISC': 40, 'WAILMER': 40, 'CORSOLA': 15, 'TATSUGIRI_CURLY': 5}),
     v_super={'BARBARACLE': 1})
area('ROUTE129', water=sea('TENTACOOL', 'WAILMER', 'VELUZA', 'WINGULL', 'PELIPPER', **LATE),
     old=OLD_SEA, good=(15, 30, {'WAILMER': 60, 'TENTACOOL': 20, 'MAGIKARP': 20}),
     super=(30, 45, {'WAILMER': 40, 'SHARPEDO': 40, 'WAILORD': 15, 'VELUZA': 5}))
area('ROUTE130', water=sea('TENTACOOL', 'WINGULL', 'PYUKUMUKU', 'PELIPPER', 'WAILMER', **LATE),
     v_water={'LAPRAS': 1},
     old=OLD_SEA, good=(15, 30, {'TENTACOOL': 60, 'WAILMER': 20, 'PYUKUMUKU': 20}),
     super=(30, 45, {'WAILMER': 40, 'SHARPEDO': 40, 'WAILORD': 15, 'TENTACRUEL': 5}))
area('ROUTE131', water=sea('TENTACOOL', 'WINGULL', 'DUCKLETT', 'PELIPPER', 'SWANNA', **LATE),
     old=OLD_SEA, good=(15, 30, {'TENTACOOL': 60, 'WAILMER': 20, 'WISHIWASHI_SOLO': 20}),
     super=(30, 45, {'WAILMER': 40, 'SHARPEDO': 40, 'WISHIWASHI_SOLO': 15, 'TENTACRUEL': 5}),
     v_super={'WISHIWASHI': 1})
area('ROUTE132', water=sea('TENTACOOL', 'WINGULL', 'CLOBBOPUS', 'PELIPPER', 'GRAPPLOCT', **LATE),
     v_water={'PINCURCHIN': 1},
     old=OLD_SEA, good=(15, 30, {'HORSEA': 60, 'TENTACOOL': 20, 'WAILMER': 20}),
     super=(30, 45, {'HORSEA': 40, 'SHARPEDO': 40, 'SEADRA': 15, 'WAILMER': 5}),
     v_super={'QWILFISH_HISUI': 1})
area('ROUTE133', water=sea('TENTACOOL', 'ARROKUDA', 'WINGULL', 'PELIPPER', 'BARRASKEWDA', **LATE),
     old=OLD_SEA, good=(15, 30, {'HORSEA': 60, 'TENTACOOL': 20, 'ARROKUDA': 20}),
     super=(30, 45, {'HORSEA': 40, 'SHARPEDO': 40, 'SEADRA': 15, 'BARRASKEWDA': 5}))
area('ROUTE134', water=sea('TENTACOOL', 'WINGULL', 'HORSEA', 'PELIPPER', 'RELICANTH', **LATE),
     old=OLD_SEA, good=(15, 30, {'HORSEA': 60, 'TENTACOOL': 20, 'WAILMER': 20}),
     super=(30, 45, {'HORSEA': 40, 'SHARPEDO': 40, 'SEADRA': 15, 'WAILMER': 5}))

# ---------------------------------------------------------------- towns and cities with water
area('PETALBURG_CITY', water=(20, 30, {'MARILL': 35, 'LOTAD': 30, 'PSYDUCK': 20, 'GOLDEEN': 10, 'AZUMARILL': 5}),
     old=OLD_FRESH, good=(10, 25, {'GOLDEEN': 60, 'CORPHISH': 20, 'MAGIKARP': 20}),
     super=(20, 35, {'CORPHISH': 40, 'GOLDEEN': 40, 'CRAWDAUNT': 15, 'SEAKING': 5}))
area('DEWFORD_TOWN', water=sea('TENTACOOL', 'WINGULL', 'WIGLETT', 'PELIPPER', 'KRABBY'),
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'KRABBY': 20, 'WAILMER': 20}),
     super=(25, 40, {'WAILMER': 40, 'KINGLER': 40, 'SHARPEDO': 15, 'WUGTRIO': 5}))
area('SLATEPORT_CITY', water=sea('TENTACOOL', 'WINGULL', 'SHELLOS', 'PELIPPER', 'WIGLETT'),
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'WAILMER': 20, 'KRABBY': 20}),
     super=(25, 40, {'WAILMER': 40, 'SHARPEDO': 40, 'KINGLER': 15, 'TENTACRUEL': 5}))
area('LILYCOVE_CITY', water=sea('TENTACOOL', 'WINGULL', 'STARYU', 'PELIPPER', 'CRAMORANT'),
     old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'STARYU': 20, 'WAILMER': 20}),
     super=(25, 45, {'WAILMER': 40, 'STARYU': 40, 'STARMIE': 15, 'WAILORD': 5}))
area('MOSSDEEP_CITY', water=sea('TENTACOOL', 'WINGULL', 'MANTYKE', 'PELIPPER', 'MANTINE', **LATE),
     old=OLD_SEA, good=(15, 30, {'TENTACOOL': 60, 'WAILMER': 20, 'REMORAID': 20}),
     super=(30, 45, {'WAILMER': 40, 'SHARPEDO': 40, 'REMORAID': 15, 'CLAWITZER': 5}))
area('PACIFIDLOG_TOWN', water=sea('TENTACOOL', 'WINGULL', 'CORSOLA', 'PELIPPER', 'SHELLDER', **LATE),
     old=OLD_SEA, good=(15, 30, {'TENTACOOL': 60, 'SHELLDER': 20, 'WAILMER': 20}),
     super=(30, 45, {'WAILMER': 40, 'SHARPEDO': 40, 'CLOYSTER': 15, 'CORSOLA': 5}))
area('EVER_GRANDE_CITY', water=sea('TENTACOOL', 'WINGULL', 'LUVDISC', 'PELIPPER', 'TENTACRUEL', **LATE),
     v_water={'DONDOZO': 1},
     old=OLD_SEA, good=(15, 30, {'LUVDISC': 60, 'TENTACOOL': 20, 'WAILMER': 20}),
     super=(30, 45, {'LUVDISC': 40, 'WAILMER': 40, 'CORSOLA': 15, 'WAILORD': 5}))
area('SOOTOPOLIS_CITY', water=(30, 40, {'MAGIKARP': 35, 'GOLDEEN': 30, 'STARYU': 20, 'GYARADOS': 10, 'SEAKING': 5}),
     v_water={'STARMIE': 1},
     old=(5, 10, {'MAGIKARP': 70, 'GOLDEEN': 30}), good=(15, 30, {'MAGIKARP': 60, 'GOLDEEN': 20, 'STARYU': 20}),
     super=(30, 45, {'MAGIKARP': 40, 'GOLDEEN': 40, 'GYARADOS': 15, 'SEAKING': 5}))

# ---------------------------------------------------------------- underwater
area('UNDERWATER_ROUTE124', water=(20, 35, {'CLAMPERL': 35, 'CHINCHOU': 30, 'LUVDISC': 20, 'CORSOLA': 10, 'RELICANTH': 5}),
     v_water={'HUNTAIL': 1})
area('UNDERWATER_ROUTE126', water=(20, 35, {'CLAMPERL': 35, 'CHINCHOU': 30, 'HORSEA': 20, 'CORSOLA_GALAR': 10, 'RELICANTH': 5}),
     v_water={'GOREBYSS': 1})

# ================================================================= FORESTS AND CAVES
area('PETALBURG_WOODS', land=(5, 6, {'WURMPLE': 20, 'SHROOMISH': 20, 'POOCHYENA': 10, 'CATERPIE': 10, 'WEEDLE': 10, 'TAILLOW': 10,
                                     'SLAKOTH': 5, 'SEWADDLE': 5, 'BURMY': 4, 'COMBEE': 4, 'PARAS': 1, 'TREECKO': 1}),
     v_land={'MORELULL': 2, 'BLIPBUG': 2, 'VENIPEDE': 2, 'SCATTERBUG': 1})

area('RUSTURF_TUNNEL', land=(5, 8, {'WHISMUR': 40, 'ZUBAT': 10, 'DIGLETT': 10, 'ROGGENROLA': 10, 'WOOBAT': 10,
                                    'TIMBURR': 5, 'BONSLY': 5, 'DRILBUR': 4, 'GEODUDE': 4, 'NOSEPASS': 1, 'ONIX': 1}))

area('GRANITE_CAVE_1F', land=(7, 10, {'ZUBAT': 25, 'MAKUHITA': 20, 'GEODUDE': 15, 'ABRA': 10, 'ONIX': 10, 'DURANT': 10,
                                      'NOIBAT': 5, 'WOOBAT': 4, 'BRONZOR': 1}))
area('GRANITE_CAVE_B1F', land=(9, 11, {'ARON': 25, 'ZUBAT': 25, 'MAKUHITA': 10, 'ABRA': 10, 'SABLEYE': 10, 'GEODUDE': 10,
                                       'NOIBAT': 5, 'CUFANT': 4, 'BRONZOR': 1}),
     v_land={'DUNSPARCE': 1})
area('GRANITE_CAVE_B2F', land=(10, 12, {'ARON': 25, 'ZUBAT': 25, 'SABLEYE': 15, 'ABRA': 10, 'GEODUDE': 10, 'ONIX': 5,
                                        'CARBINK': 4, 'CUFANT': 4, 'DURANT': 1, 'NOSEPASS': 1}),
     rock=(10, 20, {'GEODUDE': 40, 'NOSEPASS': 30, 'ROGGENROLA': 15, 'CARBINK': 10, 'GRAVELER': 5}),
     v_land={'FERROSEED': 1},
     v_rock={'LILEEP': 1})
area('GRANITE_CAVE_STEVENS_ROOM', land=(6, 10, {'MAKUHITA': 30, 'ZUBAT': 30, 'ARON': 20, 'ABRA': 10, 'SABLEYE': 5, 'GEODUDE': 4, 'BELDUM': 1}))

area('FIERY_PATH', land=(14, 16, {'NUMEL': 20, 'KOFFING': 15, 'SLUGMA': 15, 'TORKOAL': 10, 'MACHOP': 10, 'GRIMER': 5,
                                  'GROWLITHE': 5, 'SALANDIT': 5, 'TURTONATOR': 4, 'SIZZLIPEDE': 4, 'TEPIG': 1}))

area('JAGGED_PASS', land=(20, 22, {'NUMEL': 20, 'MACHOP': 20, 'SPOINK': 15, 'GEODUDE': 10, 'PANSEAR': 10, 'SCRAGGY': 5,
                                   'HEATMOR': 5, 'SAWK': 4, 'THROH': 4, 'CYNDAQUIL': 1, 'SKARMORY': 1}),
     v_land={'DIGLETT_ALOLA': 1, 'GEODUDE_ALOLA': 1, 'TINKATINK': 1})

FALLS_OLD = OLD_FRESH
FALLS_GOOD = (10, 25, {'GOLDEEN': 60, 'BARBOACH': 20, 'MAGIKARP': 20})
FALLS_SUPER = (25, 40, {'BARBOACH': 40, 'GOLDEEN': 40, 'WHISCASH': 15, 'SEAKING': 5})
area('METEOR_FALLS_1F_1R', land=(14, 20, {'ZUBAT': 35, 'SOLROCK': 20, 'MINIOR': 15, 'LUNATONE': 10, 'WOOBAT': 10, 'CLEFAIRY': 4,
                                          'ROGGENROLA': 4, 'TYRUNT': 1, 'SOLROCK ': 1}),
     water=(15, 30, {'ZUBAT': 35, 'GOLDEEN': 30, 'BARBOACH': 20, 'PSYDUCK': 10, 'SOLROCK': 5}),
     old=FALLS_OLD, good=FALLS_GOOD, super=FALLS_SUPER)
area('METEOR_FALLS_1F_2R', land=(33, 40, {'GOLBAT': 30, 'SOLROCK': 25, 'LUNATONE': 10, 'MINIOR': 10, 'DRUDDIGON': 5, 'CLEFAIRY': 5,
                                          'BOLDORE': 4, 'CRANIDOS': 1, 'SHELGON': 1}),
     v_land={'CLEFFA': 0.5},
     water=(30, 40, {'GOLBAT': 35, 'GOLDEEN': 30, 'WHISCASH': 20, 'GOLDUCK': 10, 'SOLROCK': 5}),
     old=FALLS_OLD, good=FALLS_GOOD, super=FALLS_SUPER)
area('METEOR_FALLS_B1F_1R', land=(33, 40, {'GOLBAT': 35, 'SOLROCK': 25, 'LUNATONE': 10, 'MINIOR': 10, 'BOLDORE': 10, 'DRUDDIGON': 4,
                                           'SHIELDON': 1, 'GABITE': 1}),
     water=(30, 40, {'GOLBAT': 35, 'GOLDEEN': 30, 'WHISCASH': 20, 'GOLDUCK': 10, 'SOLROCK': 5}),
     old=FALLS_OLD, good=FALLS_GOOD, super=FALLS_SUPER)
area('METEOR_FALLS_B1F_2R', land=(30, 40, {'GOLBAT': 35, 'SOLROCK': 25, 'LUNATONE': 10, 'MINIOR': 10, 'BOLDORE': 10, 'DRUDDIGON': 4,
                                           'BAGON': 1, 'GIBLE': 1}),
     v_land={'ARCHEN': 1},
     water=(30, 40, {'GOLBAT': 35, 'GOLDEEN': 30, 'WHISCASH': 20, 'GOLDUCK': 10, 'SOLROCK': 5}),
     old=FALLS_OLD, good=FALLS_GOOD, super=FALLS_SUPER)
area('METEOR_FALLS_STEVENS_CAVE', land=(33, 40, {'GOLBAT': 35, 'SOLROCK': 25, 'LUNATONE': 15, 'MINIOR': 10, 'BRONZONG': 10, 'DRAMPA': 4, 'DRUDDIGON': 1}),
     v_land={'METANG': 1})

# Mt. Pyre: Shuppet and Duskull still lead every floor, but other ghosts of graves, candles and old
# spirits now share it. Trade/item evolutions (Gourgeist, Polteageist) are left to evolution.
area('MT_PYRE_1F', land=(22, 29, {'SHUPPET': 30, 'DUSKULL': 25, 'LITWICK': 15, 'GASTLY': 10, 'PUMPKABOO_AVERAGE': 10, 'GREAVARD': 5,
                                  'SINISTEA_PHONY': 4, 'GOTHITA': 1}),
     v_land={'PUMPKABOO_SMALL': 2})
area('MT_PYRE_2F', land=(22, 29, {'SHUPPET': 30, 'DUSKULL': 25, 'DRIFLOON': 15, 'LITWICK': 10, 'GASTLY': 10, 'SINISTEA_PHONY': 5,
                                  'GREAVARD': 4, 'SINISTEA': 1}),
     v_land={'SINISTEA_ANTIQUE': 0.2, 'POLTCHAGEIST_ARTISAN': 0.2})
area('MT_PYRE_3F', land=(22, 29, {'SHUPPET': 30, 'DUSKULL': 25, 'YAMASK': 15, 'DRIFLOON': 10, 'LITWICK': 10, 'GASTLY': 5,
                                  'PHANTUMP': 4, 'SPIRITOMB': 1}))
area('MT_PYRE_4F', land=(22, 29, {'SHUPPET': 30, 'DUSKULL': 25, 'MISDREAVUS': 15, 'YAMASK': 10, 'GASTLY': 10, 'HAUNTER': 5,
                                  'LAMPENT': 4, 'MIMIKYU_DISGUISED': 1}),
     v_land={'GIMMIGHOUL_CHEST': 0.5})
area('MT_PYRE_5F', land=(22, 29, {'SHUPPET': 30, 'DUSKULL': 25, 'MISDREAVUS': 15, 'HONEDGE': 10, 'HAUNTER': 10, 'GREAVARD': 5,
                                  'DRIFBLIM': 4, 'SPIRITOMB': 1}))
area('MT_PYRE_6F', land=(22, 29, {'SHUPPET': 30, 'DUSKULL': 30, 'HONEDGE': 10, 'MISDREAVUS': 10, 'HAUNTER': 10, 'LAMPENT': 5,
                                  'DRIFBLIM': 4, 'POLTEAGEIST': 1}))
area('MT_PYRE_EXTERIOR', land=(25, 29, {'SHUPPET': 20, 'VULPIX': 20, 'MURKROW': 10, 'PHANTUMP': 10, 'CUBONE': 10, 'MEDITITE': 10,
                                        'DUSKULL': 5, 'BOMBIRDIER': 5, 'NATU': 4, 'PUMPKABOO': 4, 'ZORUA_HISUI': 1}),
     v_land={'PUMPKABOO_LARGE': 2, 'PUMPKABOO_SUPER': 0.5})
area('MT_PYRE_SUMMIT', land=(25, 30, {'SHUPPET': 25, 'DUSKULL': 20, 'MURKROW': 10, 'VULLABY': 10, 'ELGYEM': 10, 'NATU': 10,
                                      'CHIMECHO': 5, 'DUSCLOPS': 4, 'DREEPY': 1}))

# Seafloor Cavern: sea-cave natives join the bats; fossils stay 1%.
SF = {'ZUBAT': 30, 'GOLBAT': 20, 'BINACLE': 10, 'DWEBBLE': 10, 'SANDYGAST': 10}
area('SEAFLOOR_CAVERN_ROOM1', land=(28, 36, dict(SF, CORPHISH=10, STUNFISK=5, KABUTO=1)))
area('SEAFLOOR_CAVERN_ROOM2', land=(28, 36, dict(SF, CORPHISH=10, STUNFISK=5, GLIMMET=4, OMANYTE=1)))
area('SEAFLOOR_CAVERN_ROOM3', land=(28, 36, dict(SF, STUNFISK=10, GLIMMET=5, CRUSTLE=4, TIRTOUGA=1)))
area('SEAFLOOR_CAVERN_ROOM4', land=(28, 36, dict(SF, STUNFISK=10, CORPHISH=5, BARBARACLE=4, KABUTO=1)))
area('SEAFLOOR_CAVERN_ROOM5', land=(28, 36, dict(SF, CORPHISH=10, INKAY=5, PALOSSAND=4, ARCTOZOLT=1)))
SFW = dict(water=(25, 35, {'TENTACOOL': 35, 'ZUBAT': 30, 'CHINCHOU': 20, 'GOLBAT': 10, 'TENTACRUEL': 5}),
           old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'CHINCHOU': 20, 'WAILMER': 20}),
           super=(25, 40, {'WAILMER': 40, 'LANTURN': 40, 'SHARPEDO': 15, 'TENTACRUEL': 5}))
area('SEAFLOOR_CAVERN_ROOM6', land=(28, 36, dict(SF, INKAY=10, STUNFISK=5, GLIMMET=4, OMANYTE=1)), **SFW,
     v_super={'DRACOVISH': 1})
area('SEAFLOOR_CAVERN_ROOM7', land=(28, 36, dict(SF, INKAY=10, CORPHISH=5, CRUSTLE=4, TIRTOUGA=1)), **SFW,
     v_super={'ARCTOVISH': 1})
area('SEAFLOOR_CAVERN_ROOM8', land=(28, 36, dict(SF, CORPHISH=10, INKAY=5, GLIMMET=4, OMANYTE=1)))
area('SEAFLOOR_CAVERN_ENTRANCE', **SFW)

COO = {'ZUBAT': 30, 'SABLEYE': 25, 'GOLBAT': 20, 'BRONZOR': 10, 'GOLETT': 10, 'CARBINK': 4, 'BRONZONG': 1}
for m in ('CAVE_OF_ORIGIN_ENTRANCE', 'CAVE_OF_ORIGIN_1F', 'CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP1',
          'CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP2', 'CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP3'):
    area(m, land=(30, 36, dict(COO)))

area('NEW_MAUVILLE_ENTRANCE', land=(22, 26, {'VOLTORB': 30, 'MAGNEMITE': 30, 'KLINK': 15, 'TYNAMO': 10, 'JOLTIK': 10, 'VAROOM': 4, 'ROTOM': 1}),
     v_land={'ELEKID': 1})
area('NEW_MAUVILLE_INSIDE', land=(22, 26, {'MAGNEMITE': 30, 'VOLTORB': 30, 'KLINK': 15, 'TYNAMO': 10, 'JOLTIK': 5, 'DEDENNE': 5,
                                           'TOXEL': 1, 'ROTOM': 1}),
     v_land={'KLEFKI': 2, 'PORYGON': 0.5, 'VOLTORB_HISUI': 1, 'BURMY_TRASH': 2, 'ELEKID': 1})

VR1 = {'GOLBAT': 20, 'HARIYAMA': 15, 'LAIRON': 15, 'LOUDRED': 10, 'MEDICHAM': 10, 'MAKUHITA': 5,
       'MIENFOO': 5, 'PAWNIARD': 5, 'AXEW': 4, 'ARON': 4, 'RIOLU': 1}
area('VICTORY_ROAD_1F', land=(36, 42, VR1))
area('VICTORY_ROAD_B1F', land=(38, 42, {'GOLBAT': 25, 'HARIYAMA': 20, 'LAIRON': 15, 'MAWILE': 10, 'BOLDORE': 10, 'EXCADRILL': 5,
                                        'FRAXURE': 5, 'PAWNIARD': 5, 'DURALUDON': 4, 'DEINO': 1, 'MEDICHAM': 1}),
     v_land={'TYROGUE': 1},
     rock=(30, 40, {'GRAVELER': 40, 'GEODUDE': 30, 'BOLDORE': 15, 'CARKOL': 10, 'ROGGENROLA': 5}),
     v_rock={'AERODACTYL': 1})
area('VICTORY_ROAD_B2F', land=(40, 44, {'GOLBAT': 25, 'SABLEYE': 20, 'LAIRON': 15, 'MAWILE': 10, 'HARIYAMA': 10, 'FRAXURE': 10,
                                        'AGGRON': 4, 'DURALUDON': 4, 'JANGMO_O': 1, 'RIOLU': 1}),
     v_land={'HAKAMO_O': 1},
     water=(30, 40, {'GOLBAT': 35, 'GOLDEEN': 30, 'WHISCASH': 20, 'SEAKING': 10, 'BARBOACH': 5}),
     old=FALLS_OLD, good=FALLS_GOOD, super=FALLS_SUPER)

SHOAL_W = dict(water=(25, 35, {'TENTACOOL': 35, 'SPHEAL': 30, 'SEEL': 20, 'ZUBAT': 10, 'SEALEO': 5}),
               old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'SHELLDER': 20, 'WAILMER': 20}),
               super=(25, 40, {'WAILMER': 40, 'SHELLDER': 40, 'CLOYSTER': 15, 'DEWGONG': 5}))
area('SHOAL_CAVE_LOW_TIDE_ENTRANCE_ROOM', land=(26, 32, {'SPHEAL': 30, 'ZUBAT': 25, 'SNORUNT': 10, 'CRYOGONAL': 10, 'SWINUB': 10,
                                                         'GOLBAT': 5, 'BERGMITE': 5, 'SNOM': 4, 'EISCUE': 1}), **SHOAL_W)
area('SHOAL_CAVE_LOW_TIDE_INNER_ROOM', land=(26, 32, {'SPHEAL': 30, 'ZUBAT': 25, 'SWINUB': 10, 'SNORUNT': 10, 'SEEL': 10,
                                                      'GOLBAT': 5, 'VANILLITE': 5, 'SLOWPOKE': 4, 'SMOOCHUM': 1}), **SHOAL_W)
area('SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM', land=(26, 32, {'ZUBAT': 30, 'SPHEAL': 25, 'SNORUNT': 10, 'VANILLITE': 10, 'SNOM': 10,
                                                       'SNEASEL_HISUI': 5, 'GOLBAT': 5, 'BERGMITE': 4, 'AMAURA': 1}))
area('SHOAL_CAVE_LOW_TIDE_LOWER_ROOM', land=(26, 32, {'SPHEAL': 30, 'ZUBAT': 25, 'SNORUNT': 10, 'BERGMITE': 10, 'SWINUB': 10,
                                                      'GLALIE': 5, 'GOLBAT': 5, 'CRYOGONAL': 4, 'AMAURA': 1}),
     v_land={'SMOOCHUM': 1})
area('SHOAL_CAVE_LOW_TIDE_ICE_ROOM', land=(26, 32, {'SNORUNT': 20, 'SPHEAL': 20, 'SNEASEL': 10, 'DELIBIRD': 10, 'CUBCHOO': 10,
                                                    'SNOVER': 10, 'ZUBAT': 5, 'SWINUB': 5, 'CETODDLE': 4, 'SNOM': 4, 'FRIGIBAX': 1}),
     v_land={'EISCUE_ICE': 1, 'SANDSHREW_ALOLA': 1, 'VULPIX_ALOLA': 0.5, 'DARUMAKA_GALAR': 0.5,
             'VIVILLON_POLAR': 0.5, 'VIVILLON_TUNDRA': 0.5, 'VIVILLON_ICY_SNOW': 0.5})

SKY = {'GOLBAT': 25, 'SABLEYE': 20, 'CLAYDOL': 15, 'BANETTE': 10, 'SIGILYPH': 10, 'NOIBAT': 5, 'RUFFLET': 5, 'BRONZOR': 4}
area('SKY_PILLAR_1F', land=(33, 38, dict(SKY, BRONZONG=4, DRAMPA=1)), v_land={'DRAGONAIR': 1})
area('SKY_PILLAR_3F', land=(33, 38, dict(SKY, ALTARIA=4, DRAMPA=1)), v_land={'SOLOSIS': 1})
area('SKY_PILLAR_5F', land=(33, 39, dict(SKY, ALTARIA=4, DRAMPA=1)), v_land={'DRAGONAIR': 1})

MAGMA = {'GEODUDE': 20, 'TORKOAL': 20, 'SLUGMA': 10, 'GRAVELER': 10, 'NUMEL': 10, 'CARKOL': 10, 'MAGMAR': 5, 'CENTISKORCH': 5}
area('MAGMA_HIDEOUT_1F', land=(27, 33, dict(MAGMA, HEATMOR=4, LARVESTA=1)))
area('MAGMA_HIDEOUT_2F_1R', land=(27, 33, dict(MAGMA, HEATMOR=4, CAMERUPT=1)))
area('MAGMA_HIDEOUT_2F_2R', land=(27, 33, dict(MAGMA, MAGCARGO=4, LARVESTA=1)))
area('MAGMA_HIDEOUT_2F_3R', land=(27, 33, dict(MAGMA, HEATMOR=4, DRACOZOLT=1)))
area('MAGMA_HIDEOUT_3F_1R', land=(27, 33, dict(MAGMA, MAGCARGO=4, PUPITAR=1)))
area('MAGMA_HIDEOUT_3F_2R', land=(27, 33, dict(MAGMA, HEATMOR=4, CAMERUPT=1)))
area('MAGMA_HIDEOUT_3F_3R', land=(27, 33, dict(MAGMA, MAGCARGO=4, LARVESTA=1)))
area('MAGMA_HIDEOUT_4F', land=(27, 33, dict(MAGMA, MAGCARGO=4, PUPITAR=1)))

MT = {'SANDSHREW': 30, 'TRAPINCH': 30, 'BALTOY': 15, 'YAMASK_GALAR': 10}
area('MIRAGE_TOWER_1F', land=(20, 24, dict(MT, FLITTLE=10, NACLI=4, GIMMIGHOUL_ROAMING=1)))
area('MIRAGE_TOWER_2F', land=(20, 24, dict(MT, NACLI=10, FLITTLE=4, GIMMIGHOUL_ROAMING=1)))
area('MIRAGE_TOWER_3F', land=(20, 24, dict(MT, BRAMBLIN=10, FLITTLE=4, SIGILYPH=1)))
area('MIRAGE_TOWER_4F', land=(20, 24, dict(MT, BRAMBLIN=10, NACLI=4, SIGILYPH=1)))

area('DESERT_UNDERPASS', land=(35, 45, {'DITTO': 30, 'LOUDRED': 15, 'WHISMUR': 10, 'YAMASK_GALAR': 10, 'SANDACONDA': 10,
                                        'DUGTRIO': 10, 'RELLOR': 5, 'DUDUNSPARCE': 4, 'EXPLOUD': 4, 'MEOWTH_GALAR': 1, 'DITTO ': 1}))

# ================================================================= SAFARI ZONE
area('SAFARI_ZONE_SOUTH', land=(25, 29, {'ODDISH': 20, 'GIRAFARIG': 15, 'DODUO': 15, 'NATU': 10, 'NIDORAN_F': 10, 'NIDORAN_M': 10,
                                         'TAUROS': 5, 'BOUNSWEET': 5, 'DEERLING': 4, 'MAREEP': 4, 'BULBASAUR': 1, 'PIKACHU': 1}),
     v_land={'TAUROS_PALDEA_COMBAT': 1, 'BOUFFALANT': 3, 'MUDBRAY': 3})
area('SAFARI_ZONE_SOUTHWEST', land=(25, 29, {'ODDISH': 20, 'GIRAFARIG': 15, 'WOBBUFFET': 10, 'NATU': 10, 'DODUO': 10, 'PIKACHU': 10,
                                             'CROAGUNK': 5, 'YUNGOOS': 5, 'PSYDUCK': 4, 'NIDORINA': 4, 'TURTWIG': 1, 'NIDORINO': 1}),
     v_land={'TAUROS_PALDEA_BLAZE': 0.5, 'BURMY_PLANT': 2},
     water=(20, 35, {'PSYDUCK': 35, 'MARILL': 30, 'GOLDEEN': 20, 'WOOPER': 10, 'GOLDUCK': 5}),
     old=OLD_FRESH, good=(10, 25, {'GOLDEEN': 60, 'MAGIKARP': 20, 'REMORAID': 20}),
     super=(25, 40, {'GOLDEEN': 40, 'SEAKING': 40, 'REMORAID': 15, 'GYARADOS': 5}))
area('SAFARI_ZONE_NORTH', land=(27, 31, {'PHANPY': 20, 'ODDISH': 15, 'NATU': 10, 'GLOOM': 10, 'HERACROSS': 10, 'PANCHAM': 10,
                                         'XATU': 5, 'ORANGURU': 5, 'DONPHAN': 4, 'SNIVY': 1, 'APPLIN': 1}),
     v_land={'TOGEPI': 1},
     rock=(10, 25, {'GEODUDE': 40, 'ROGGENROLA': 30, 'GRAVELER': 15, 'DWEBBLE': 10, 'BOLDORE': 5}),
     v_rock={'FERROSEED': 1})
area('SAFARI_ZONE_NORTHWEST', land=(27, 31, {'RHYHORN': 20, 'ODDISH': 15, 'DODUO': 10, 'GLOOM': 10, 'LICKITUNG': 10, 'PASSIMIAN': 10,
                                             'PINSIR': 5, 'HERACROSS': 5, 'DODRIO': 4, 'GIRAFARIG': 4, 'CHESPIN': 1, 'SCYTHER': 1}),
     v_land={'VIVILLON_POKEBALL': 0.5},
     water=(20, 40, {'PSYDUCK': 35, 'GOLDEEN': 30, 'MARILL': 20, 'GOLDUCK': 10, 'SEAKING': 5}),
     old=OLD_FRESH, good=(10, 25, {'GOLDEEN': 60, 'MAGIKARP': 20, 'POLIWAG': 20}),
     super=(25, 40, {'GOLDEEN': 40, 'SEAKING': 40, 'POLIWHIRL': 15, 'GYARADOS': 5}))
area('SAFARI_ZONE_SOUTHEAST', land=(33, 40, {'SUNKERN': 15, 'MAREEP': 15, 'WOOLOO': 10, 'AIPOM': 10, 'SPINARAK': 10, 'SNUBBULL': 10,
                                             'STANTLER': 5, 'KOMALA': 5, 'GLIGAR': 4, 'PAWMI': 4, 'KANGASKHAN': 1, 'FLAAFFY': 1}),
     water=(25, 40, {'WOOPER': 35, 'MARILL': 30, 'QUAGSIRE': 20, 'AZUMARILL': 10, 'PSYDUCK': 5}),
     old=(25, 30, {'MAGIKARP': 70, 'GOLDEEN': 30}), good=(25, 35, {'REMORAID': 60, 'GOLDEEN': 20, 'MAGIKARP': 20}),
     super=(25, 40, {'REMORAID': 40, 'GOLDEEN': 40, 'OCTILLERY': 15, 'SEAKING': 5}))
area('SAFARI_ZONE_NORTHEAST', land=(33, 40, {'AIPOM': 20, 'TEDDIURSA': 15, 'SUNKERN': 10, 'LEDYBA': 10, 'HOOTHOOT': 10, 'PINECO': 10,
                                             'STUFFUL': 5, 'HOUNDOUR': 5, 'MILTANK': 5, 'URSARING': 4, 'HERACROSS': 1}),
     v_land={'SCYTHER': 2, 'TAUROS_PALDEA_AQUA': 0.5},
     rock=(20, 40, {'SHUCKLE': 40, 'GEODUDE': 30, 'GRAVELER': 15, 'DWEBBLE': 10, 'CRUSTLE': 5}))

# ================================================================= ABANDONED SHIP
SHIP = dict(water=(25, 35, {'TENTACOOL': 35, 'FRILLISH': 30, 'WINGULL': 20, 'TENTACRUEL': 10, 'JELLICENT': 5}),
            old=OLD_SEA, good=(10, 25, {'TENTACOOL': 60, 'FRILLISH': 20, 'MAGIKARP': 20}),
            super=(25, 40, {'TENTACOOL': 40, 'TENTACRUEL': 40, 'FRILLISH': 15, 'DHELMISE': 5}))
area('ABANDONED_SHIP_ROOMS_B1F', **SHIP)
area('ABANDONED_SHIP_HIDDEN_FLOOR_CORRIDORS', **SHIP)

# ---------------------------------------------------------------- Old Rod variety (audit 2026-10-23)
# Magikarp stays the Old Rod's 70% everywhere; the other 30% is the area's own small local fish
# (the first Good Rod species that isn't a generic Magikarp/Tentacool/Goldeen/Wailmer), so every
# shore and pond has its own character even with the oldest rod.
_GENERIC = {'MAGIKARP', 'TENTACOOL', 'GOLDEEN', 'WAILMER'}
for _name, _spec in D.items():
    if _spec.get('old') in (OLD_SEA, OLD_FRESH) and 'good' in _spec:
        _local = [k for k in _spec['good'][2] if k not in _GENERIC]
        if _local:
            _spec['old'] = (5, 10, {'MAGIKARP': 70, _local[0]: 30})

# ---------------------------------------------------------------- no-slot-limit clean-up (2026-10-26, owner approved)
import os as _os, sys as _sys
_sys.path.insert(0, _os.path.dirname(_os.path.abspath(__file__)) if '__file__' in globals() else 'tools')
import wally_nocap_pass as _nocap
_nocap.apply(D)
