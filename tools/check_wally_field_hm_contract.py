#!/usr/bin/env python3
"""Static guard against reverting Wally's Journey party-free field HMs.

This does NOT simulate overworld interaction; an emulator must still prove it.
"""
from pathlib import Path

def require(condition, message):
    if not condition:
        raise AssertionError(message)

def read(path):
    return Path(path).read_text(encoding="utf-8")

field = read("src/field_move.c")
scrcmd = read("src/scrcmd.c")
scripts = read("data/scripts/field_move_scripts.inc")
roxanne = read("data/maps/RustboroCity_Gym/scripts.inc")
config = read("include/config/item.h")
party = read("src/party_menu.c")

require("#define I_REUSABLE_TMS          TRUE" in config, "TMs must stay reusable.")
lic = field[field.index("bool32 WallyHasFieldMoveLicense("):field.index("bool32 WallyCanUseHmWithoutTeaching(")]
require("CheckBagHasItem(item, 1)" in lic and "IsFieldMoveUnlocked(fieldMove)" in lic,
        "Badge and HM item must license field moves.")
can_use = field[field.index("bool32 WallyCanUseHmWithoutTeaching("):field.index("bool32 WallyPartyCanUseHmWithoutTeaching(")]
require("CanLearnTeachableMove" not in can_use,
        "Do not restore HM learnset compatibility restrictions.")
start = scrcmd.index("bool8 ScrCmd_checkfieldmove(")
end = scrcmd.index("bool8 ScrCmd_addmoney(", start)
check = scrcmd[start:end]
require("if (WallyHasFieldMoveLicense(fieldMove))" in check,
        "Object interactions must recognize badge/HM licenses.")
require(check.index("if (WallyHasFieldMoveLicense(fieldMove))") < check.index("MonKnowsMove("),
        "HM license handling must not depend on a learned move.")
for move, script in [
    ("CUT", "EventScript_CutTree::"),
    ("ROCK_SMASH", "EventScript_RockSmash::"),
    ("STRENGTH", "EventScript_StrengthBoulder::"),
    ("WATERFALL", "EventScript_UseWaterfall::"),
    ("DIVE", "EventScript_UseDive::"),
]:
    block = scripts[scripts.index(script):]
    block = block[:block.index("\n\n")]
    require(f"checkfieldmove FIELD_MOVE_{move}" in block,
            f"Object/script interaction missing for {move}.")
require("FIELD_MOVE_FLY" in party[party.index("sLicensedHmPriority"):party.index("// Reserve three slots")],
        "Fly must remain in the prioritized licensed action list.")
require("a TM can be used only once" not in roxanne,
        "Roxanne must not claim TMs are consumed.")
require("No POKéMON needs to learn CUT" in roxanne,
        "Roxanne's Cut advice must not require a compatible Pokémon.")
print("PASS: field HM license and reusable-TM static contracts.")
