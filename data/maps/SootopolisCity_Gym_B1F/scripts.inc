SootopolisCity_Gym_B1F_MapScripts::
	.byte 0

SootopolisCity_Gym_B1F_EventScript_Andrea::
	trainerbattle_single TRAINER_ANDREA, SootopolisCity_Gym_B1F_Text_AndreaIntro, SootopolisCity_Gym_B1F_Text_AndreaDefeat
	msgbox SootopolisCity_Gym_B1F_Text_AndreaPostBattle, MSGBOX_AUTOCLOSE
	end

SootopolisCity_Gym_B1F_EventScript_Crissy::
	trainerbattle_single TRAINER_CRISSY, SootopolisCity_Gym_B1F_Text_CrissyIntro, SootopolisCity_Gym_B1F_Text_CrissyDefeat
	msgbox SootopolisCity_Gym_B1F_Text_CrissyPostBattle, MSGBOX_AUTOCLOSE
	end

SootopolisCity_Gym_B1F_EventScript_Brianna::
	trainerbattle_single TRAINER_BRIANNA, SootopolisCity_Gym_B1F_Text_BriannaIntro, SootopolisCity_Gym_B1F_Text_BriannaDefeat
	msgbox SootopolisCity_Gym_B1F_Text_BriannaPostBattle, MSGBOX_AUTOCLOSE
	end

SootopolisCity_Gym_B1F_EventScript_Connie::
	trainerbattle_single TRAINER_CONNIE, SootopolisCity_Gym_B1F_Text_ConnieIntro, SootopolisCity_Gym_B1F_Text_ConnieDefeat
	msgbox SootopolisCity_Gym_B1F_Text_ConniePostBattle, MSGBOX_AUTOCLOSE
	end

SootopolisCity_Gym_B1F_EventScript_Bridget::
	trainerbattle_single TRAINER_BRIDGET, SootopolisCity_Gym_B1F_Text_BridgetIntro, SootopolisCity_Gym_B1F_Text_BridgetDefeat
	msgbox SootopolisCity_Gym_B1F_Text_BridgetPostBattle, MSGBOX_AUTOCLOSE
	end

SootopolisCity_Gym_B1F_EventScript_Olivia::
	trainerbattle_single TRAINER_OLIVIA, SootopolisCity_Gym_B1F_Text_OliviaIntro, SootopolisCity_Gym_B1F_Text_OliviaDefeat
	msgbox SootopolisCity_Gym_B1F_Text_OliviaPostBattle, MSGBOX_AUTOCLOSE
	end

SootopolisCity_Gym_B1F_EventScript_Tiffany::
	trainerbattle_single TRAINER_TIFFANY, SootopolisCity_Gym_B1F_Text_TiffanyIntro, SootopolisCity_Gym_B1F_Text_TiffanyDefeat
	msgbox SootopolisCity_Gym_B1F_Text_TiffanyPostBattle, MSGBOX_AUTOCLOSE
	end

SootopolisCity_Gym_B1F_EventScript_Bethany::
	trainerbattle_single TRAINER_BETHANY, SootopolisCity_Gym_B1F_Text_BethanyIntro, SootopolisCity_Gym_B1F_Text_BethanyDefeat
	msgbox SootopolisCity_Gym_B1F_Text_BethanyPostBattle, MSGBOX_AUTOCLOSE
	end

SootopolisCity_Gym_B1F_EventScript_Annika::
	trainerbattle_single TRAINER_ANNIKA, SootopolisCity_Gym_B1F_Text_AnnikaIntro, SootopolisCity_Gym_B1F_Text_AnnikaDefeat
	msgbox SootopolisCity_Gym_B1F_Text_AnnikaPostBattle, MSGBOX_AUTOCLOSE
	end

SootopolisCity_Gym_B1F_EventScript_Daphne::
	trainerbattle_single TRAINER_DAPHNE, SootopolisCity_Gym_B1F_Text_DaphneIntro, SootopolisCity_Gym_B1F_Text_DaphneDefeat
	msgbox SootopolisCity_Gym_B1F_Text_DaphnePostBattle, MSGBOX_AUTOCLOSE
	end

SootopolisCity_Gym_B1F_Text_AndreaIntro:
	.string "JUAN taught me to dance on ice\n"
	.string "before he taught me to battle.\p"
	.string "Watch my footwork!$"

SootopolisCity_Gym_B1F_Text_AndreaDefeat:
	.string "I slipped… JUAN will make\n"
	.string "me dance it again.$"

SootopolisCity_Gym_B1F_Text_AndreaPostBattle:
	.string "Watch what happens if you crack all\n"
	.string "the floor tiles.\p"
	.string "JUAN laid this ice himself. He calls\n"
	.string "it dancing practice.$"

SootopolisCity_Gym_B1F_Text_CrissyIntro:
	.string "JUAN only battles TRAINERS who\n"
	.string "make it through his ice.\p"
	.string "And you won't get through me!$"

SootopolisCity_Gym_B1F_Text_CrissyDefeat:
	.string "You're strong!\n"
	.string "Your cute face fooled me!$"

SootopolisCity_Gym_B1F_Text_CrissyPostBattle:
	.string "JUAN retired once. Then WALLACE left\n"
	.string "for the LEAGUE and the GYM sat empty.\p"
	.string "He came back the next morning. Said\n"
	.string "an empty GYM is a sad dance floor.$"

SootopolisCity_Gym_B1F_Text_DaphneIntro:
	.string "Watching JUAN battle…\n"
	.string "That's why I became a TRAINER.$"

SootopolisCity_Gym_B1F_Text_DaphneDefeat:
	.string "That was more graceful than\n"
	.string "anything I've ever done.$"

SootopolisCity_Gym_B1F_Text_DaphnePostBattle:
	.string "JUAN was WALLACE's teacher.\p"
	.string "He watches WALLACE's matches on TV\n"
	.string "with the sound off. Just to see the\l"
	.string "movement.$"

SootopolisCity_Gym_B1F_Text_ConnieIntro:
	.string "This ice is from the crater lake.\n"
	.string "JUAN cuts it himself every spring.\p"
	.string "Let me show you how harsh it is!$"

SootopolisCity_Gym_B1F_Text_ConnieDefeat:
	.string "Oh. The ice liked you\n"
	.string "better than me.$"

SootopolisCity_Gym_B1F_Text_ConniePostBattle:
	.string "Here's a secret.\p"
	.string "To reach JUAN, step on each floor\n"
	.string "tile exactly once.$"

SootopolisCity_Gym_B1F_Text_BridgetIntro:
	.string "The highest-level GYM in HOENN…\n"
	.string "Welcome to SOOTOPOLIS.$"

SootopolisCity_Gym_B1F_Text_BridgetDefeat:
	.string "You're stronger than anyone\n"
	.string "who's come down here in years.$"

SootopolisCity_Gym_B1F_Text_BridgetPostBattle:
	.string "My family has guarded the CAVE OF\n"
	.string "ORIGIN for generations.\p"
	.string "JUAN's family too. He's the one who\n"
	.string "taught me the old songs.$"

SootopolisCity_Gym_B1F_Text_OliviaIntro:
	.string "I train beside JUAN every morning\n"
	.string "at dawn.\p"
	.string "He's seventy. I can barely keep up.$"

SootopolisCity_Gym_B1F_Text_OliviaDefeat:
	.string "Beaten… I'll need extra\n"
	.string "dawn practice.$"

SootopolisCity_Gym_B1F_Text_OliviaPostBattle:
	.string "You have potential. Stay and train\n"
	.string "with us?\p"
	.string "…No? JUAN said you'd refuse.\n"
	.string "WALLACE refused too, at first.$"

SootopolisCity_Gym_B1F_Text_TiffanyIntro:
	.string "A graceful glide, crossing no\n"
	.string "lines… That's true beauty!$"

SootopolisCity_Gym_B1F_Text_TiffanyDefeat:
	.string "Well, excuse me?!\n"
	.string "That was MY spotlight!$"

SootopolisCity_Gym_B1F_Text_TiffanyPostBattle:
	.string "How strong you are has nothing to do\n"
	.string "with how old you are.\p"
	.string "JUAN is seventy. He out-dances all\n"
	.string "of us.$"

SootopolisCity_Gym_B1F_Text_BethanyIntro:
	.string "I came to SOOTOPOLIS for one week.\n"
	.string "That was six years ago. Oops!$"

SootopolisCity_Gym_B1F_Text_BethanyDefeat:
	.string "Oops!\n"
	.string "That one's going to stick.$"

SootopolisCity_Gym_B1F_Text_BethanyPostBattle:
	.string "This city gets under your skin.\p"
	.string "The walls, the water, the old\n"
	.string "stories. I'm never leaving.$"

SootopolisCity_Gym_B1F_Text_AnnikaIntro:
	.string "My POKéMON came from the CAVE OF\n"
	.string "ORIGIN's shore. Very rare!\p"
	.string "Want to see them?$"

SootopolisCity_Gym_B1F_Text_AnnikaDefeat:
	.string "You saw them, all right…\n"
	.string "up close and fainted.$"

SootopolisCity_Gym_B1F_Text_AnnikaPostBattle:
	.string "JUAN praised my POKéMON once.\n"
	.string "I joined the next day.\p"
	.string "Oh, if only I'd met him years ago…$"

SootopolisCity_Gym_B1F_Text_BriannaIntro:
	.string "Giggle… JUAN says a frown ruins\n"
	.string "a performance.\p"
	.string "Yours is charming, though.$"

SootopolisCity_Gym_B1F_Text_BriannaDefeat:
	.string "Oh dear. I was the one who\n"
	.string "lost my composure.$"

SootopolisCity_Gym_B1F_Text_BriannaPostBattle:
	.string "You won't lay a finger on JUAN.\p"
	.string "He's never lost to anyone but\n"
	.string "WALLACE. And WALLACE cried after.$"

