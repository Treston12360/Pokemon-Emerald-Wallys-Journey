SootopolisCity_House2_MapScripts::
	.byte 0

SootopolisCity_House2_EventScript_ExpertF::
	lock
	faceplayer
	msgbox SootopolisCity_House2_Text_DidYouKnowAboutMtPyreOrbs, MSGBOX_YESNO
	call_if_eq VAR_RESULT, YES, SootopolisCity_House2_EventScript_KnowAboutOrbs
	call_if_eq VAR_RESULT, NO, SootopolisCity_House2_EventScript_DontKnowAboutOrbs
	release
	end

SootopolisCity_House2_EventScript_KnowAboutOrbs::
	msgbox SootopolisCity_House2_Text_YesTwoOrbsSideBySide, MSGBOX_DEFAULT
	return

SootopolisCity_House2_EventScript_DontKnowAboutOrbs::
	msgbox SootopolisCity_House2_Text_OughtToVisitAndSee, MSGBOX_DEFAULT
	return

SootopolisCity_House2_Text_DidYouKnowAboutMtPyreOrbs:
	.string "Two ancient ORBS rest atop\n"
	.string "MT. PYRE. Have you heard?\p"
	.string "Our grandparents were told\n"
	.string "they belong side by side.\p"
	.string "The old stories disagree\n"
	.string "about what they actually do.$"

SootopolisCity_House2_Text_YesTwoOrbsSideBySide:
	.string "Yes. Together, as they were\n"
	.string "kept for generations.\p"
	.string "People call them keys.\n"
	.string "Others call them warnings.\p"
	.string "I wonder who first decided\n"
	.string "they could be used at all.$"

SootopolisCity_House2_Text_OughtToVisitAndSee:
	.string "Then perhaps you should\n"
	.string "visit MT. PYRE someday.\p"
	.string "Not everything worth finding\n"
	.string "was left there for us to take.$"

