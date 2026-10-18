MtPyre_2F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, CaveHole_CheckFallDownHole
	map_script MAP_SCRIPT_ON_TRANSITION, CaveHole_FixCrackedGround
	map_script MAP_SCRIPT_ON_RESUME, MtPyre_2F_SetHoleWarp
	.byte 0

MtPyre_2F_SetHoleWarp:
	setstepcallback STEP_CB_CRACKED_FLOOR
	setholewarp MAP_MT_PYRE_1F
	end

MtPyre_2F_EventScript_Woman::
	msgbox MtPyre_2F_Text_MemoriesOfSkitty, MSGBOX_NPC
	end

MtPyre_2F_EventScript_PokefanM::
	msgbox MtPyre_2F_Text_TumbledFromFloorAbove, MSGBOX_NPC
	end

MtPyre_2F_EventScript_Mark::
	trainerbattle_single TRAINER_MARK, MtPyre_2F_Text_MarkIntro, MtPyre_2F_Text_MarkDefeat
	msgbox MtPyre_2F_Text_MarkPostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_2F_EventScript_Luke::
	trainerbattle_double TRAINER_DEZ_AND_LUKE, MtPyre_2F_Text_LukeIntro, MtPyre_2F_Text_LukeDefeat, MtPyre_2F_Text_LukeNotEnoughMons
	msgbox MtPyre_2F_Text_LukePostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_2F_EventScript_Dez::
	trainerbattle_double TRAINER_DEZ_AND_LUKE, MtPyre_2F_Text_DezIntro, MtPyre_2F_Text_DezDefeat, MtPyre_2F_Text_DezNotEnoughMons
	msgbox MtPyre_2F_Text_DezPostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_2F_EventScript_Leah::
	trainerbattle_single TRAINER_LEAH, MtPyre_2F_Text_LeahIntro, MtPyre_2F_Text_LeahDefeat
	msgbox MtPyre_2F_Text_LeahPostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_2F_EventScript_Zander::
	trainerbattle_single TRAINER_ZANDER, MtPyre_2F_Text_ZanderIntro, MtPyre_2F_Text_ZanderDefeat
	msgbox MtPyre_2F_Text_ZanderPostBattle, MSGBOX_AUTOCLOSE
	end

MtPyre_2F_Text_MemoriesOfSkitty:
	.string "My SKITTY used to sleep in\n"
	.string "the warmest patch of sunlight.\p"
	.string "I still leave the curtains open\n"
	.string "at home, out of habit.\p"
	.string "It's strange what love teaches\n"
	.string "us to remember.$"

MtPyre_2F_Text_TumbledFromFloorAbove:
	.string "Ooch, ouch… There are holes in the\n"
	.string "ground here and there.\p"
	.string "I didn't notice and took a tumble from\n"
	.string "the floor above.$"

MtPyre_2F_Text_MarkIntro:
	.string "Hey! I'm hunting ghost POKéMON for my\n"
	.string "school report!\p"
	.string "Did you follow me?!$"

MtPyre_2F_Text_MarkDefeat:
	.string "Ayieeeeh! I'm sorry!\n"
	.string "Don't haunt me!$"

MtPyre_2F_Text_MarkPostBattle:
	.string "I came hoping to find a rare\n"
	.string "POKéMON among the graves.\p"
	.string "It sounded harmless when I\n"
	.string "said it to my friends.\p"
	.string "Now I'm thinking about why\n"
	.string "people come here at all.$"

MtPyre_2F_Text_LukeIntro:
	.string "LUKE: We're here on a dare.\p"
	.string "If I cream you, she'll see how cool\n"
	.string "I am. I know it!$"

MtPyre_2F_Text_LukeDefeat:
	.string "LUKE: Whoopsie!\n"
	.string "That wasn't cool at all.$"

MtPyre_2F_Text_LukePostBattle:
	.string "LUKE: We lost, but I'm still right\n"
	.string "here for you, DEZ!$"

MtPyre_2F_Text_LukeNotEnoughMons:
	.string "LUKE: Bring more POKéMON if you\n"
	.string "want to take me on.\p"
	.string "Otherwise I can't show my girl\n"
	.string "how cool I am!$"

MtPyre_2F_Text_DezIntro:
	.string "DEZ: We came here on a dare.\p"
	.string "It's so scary! But LUKE will\n"
	.string "look cool if we beat you!$"

MtPyre_2F_Text_DezDefeat:
	.string "DEZ: Waaah! LUKE, I'm scared!$"

MtPyre_2F_Text_DezPostBattle:
	.string "DEZ: It's okay! We're together.\n"
	.string "That's what matters.$"

MtPyre_2F_Text_DezNotEnoughMons:
	.string "DEZ: We're a couple, so we battle\n"
	.string "as a pair.\p"
	.string "Bring two POKéMON!$"

MtPyre_2F_Text_LeahIntro:
	.string "I tend the graves here. You don't\n"
	.string "look like a mourner…$"

MtPyre_2F_Text_LeahDefeat:
	.string "Hm. You're sturdy.\n"
	.string "The spirits approve.$"

MtPyre_2F_Text_LeahPostBattle:
	.string "My family has watched over\n"
	.string "MT. PYRE for generations.\p"
	.string "A shrine is not a treasure\n"
	.string "chest for anyone to empty.\p"
	.string "If people come here seeking\n"
	.string "power, I will stand in the way.$"

MtPyre_2F_Text_ZanderIntro:
	.string "Kiyaaah! I thought you were a ghost!\p"
	.string "I came to visit my grandpa's stone.\n"
	.string "I didn't think it'd be this scary!$"

MtPyre_2F_Text_ZanderDefeat:
	.string "Nooooo! I lost my\n"
	.string "nerve AND the match!$"

MtPyre_2F_Text_ZanderPostBattle:
	.string "I freak out whenever anything\n"
	.string "moves… I shouldn't train here…$"

