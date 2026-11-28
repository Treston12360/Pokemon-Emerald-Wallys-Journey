#!/usr/bin/env python3
"""Project Tandem T5 test scripts (WJPC_SCRIPT) for two games, A (asks) and B (answers).

python3 make_scripts.py FASTBOOT.txt OUTDIR   ->  trade_A/B.txt, edge_A/B.txt, battle_A/B.txt, teamup_A/B.txt
FASTBOOT.txt: the button presses from the title screen into the saved game (Continue).
A stands at Littleroot 10,9 facing B at 11,9 (a spot the town's walkers don't reach). "waitfor" keeps the two
games in step (sdl2.c test aid): locked = a question came up, free = back in the overworld, link = the trade
screen's link has the friend, nolink = it hasn't.
"""
import sys

fb = open(sys.argv[1]).read().strip().split('\n')
out = sys.argv[2]

def write(name, lines):
    open(f'{out}/{name}.txt', 'w').write('\n'.join(fb + lines) + '\n')

# a full trade (A gives its 1st Pokemon, B its 2nd), the trade screen again, both cancel, both walk
write('trade_A', ["wait 60", "warp 0 9 10 9", "waitfor free 20000", "wait 900", "winshot f/a00_friend.png",
    "press RIGHT 3", "wait 30", "press A 3", "wait 150", "winshot f/a01_menu.png",
    "press A 3", "wait 90", "winshot f/a02_asking.png",
    "waitfor link 20000", "wait 30", "winshot f/a03_standby.png", "wait 240", "winshot f/a04_trade_screen.png",
    "press A 3", "wait 60", "winshot f/a05_mon_menu.png", "press DOWN 3", "wait 20", "press A 3", "wait 120",
    "winshot f/a06_offered.png", "wait 360", "winshot f/a07_is_this_ok.png", "press A 3", "wait 60",
    "winshot f/a08_confirmed.png", "wait 300", "winshot f/a09_anim1.png", "wait 500", "winshot f/a10_anim2.png",
    "wait 600", "winshot f/a11_anim3.png", "wait 600", "winshot f/a12_anim4.png", "wait 900", "winshot f/a13_after.png",
    "wait 600", "winshot f/a14_menu_again.png",
    "press LEFT 3", "wait 20", "press UP 3", "wait 30", "winshot f/a15_on_cancel.png", "press A 3", "wait 40",
    "winshot f/a16_cancel_q.png", "press A 3", "wait 120", "winshot f/a17_waiting.png", "waitfor free 20000", "wait 60",
    "winshot f/a18_field.png", "press LEFT 20", "wait 60", "winshot f/a19_walked.png", "wait 60", "quit"])
write('trade_B', ["wait 60", "warp 0 9 11 9", "waitfor free 20000", "winshot f/b00.png",
    "waitfor locked 30000", "wait 150", "winshot f/b01_question.png", "press A 3", "wait 60", "winshot f/b02_answered.png",
    "waitfor link 20000", "wait 270", "winshot f/b03_trade_screen.png",
    "press RIGHT 3", "wait 20", "press A 3", "wait 60", "winshot f/b04_mon_menu.png", "press DOWN 3", "wait 20",
    "press A 3", "wait 120", "winshot f/b05_offered.png", "wait 360", "winshot f/b06_is_this_ok.png", "press A 3",
    "wait 60", "winshot f/b07_confirmed.png", "wait 300", "winshot f/b08_anim1.png", "wait 500", "winshot f/b09_anim2.png",
    "wait 600", "winshot f/b10_anim3.png", "wait 600", "winshot f/b11_anim4.png", "wait 900", "winshot f/b12_after.png",
    "wait 600", "winshot f/b13_menu_again.png", "wait 300",
    "press LEFT 3", "wait 20", "press UP 3", "wait 30", "press A 3", "wait 40", "press A 3", "wait 120",
    "winshot f/b14_cancelled.png", "waitfor free 20000", "wait 60", "winshot f/b15_field.png", "press DOWN 20", "wait 60",
    "winshot f/b16_walked.png", "wait 400000", "quit"])

# B says No; A asks again and stops waiting (B answers after: "stopped waiting"); A asks a third time, B says yes and
# quits in the trade screen: A's "connection was lost" screen
write('edge_A', ["wait 60", "warp 0 9 10 9", "waitfor free 20000", "wait 900",
    "press RIGHT 3", "wait 30", "press A 3", "wait 150", "press A 3", "wait 90", "winshot f/e01_asking.png",
    "wait 400", "winshot f/e02_said_no.png", "press A 3", "wait 60", "waitfor free 3000", "wait 200",
    "press A 3", "wait 150", "press A 3", "wait 120", "winshot f/e03_asking2.png", "press B 3", "wait 60",
    "winshot f/e04_stopped_waiting.png", "waitfor free 3000", "wait 600",
    "press A 3", "wait 150", "press A 3", "waitfor link 20000", "wait 240", "winshot f/e05_trade_screen.png",
    "wait 900", "winshot f/e06_friend_left.png", "wait 300", "winshot f/e07_error.png", "press A 3",
    "waitfor free 6000", "wait 120", "winshot f/e08_back.png", "press LEFT 20", "wait 60", "winshot f/e09_walked.png",
    "press START 3", "wait 90", "winshot f/e10_menu.png", "press B 3", "wait 60", "quit"])
write('edge_B', ["wait 60", "warp 0 9 11 9", "waitfor free 20000",
    "waitfor locked 30000", "wait 150", "winshot f/f01_question.png", "press DOWN 3", "wait 20", "press A 3", "wait 60",
    "winshot f/f02_said_no.png", "waitfor free 3000", "waitfor locked 30000", "wait 150", "winshot f/f03_question2.png",
    "wait 250", "press A 3", "wait 90", "winshot f/f04_gave_up.png", "press A 3", "wait 60", "waitfor free 3000",
    "waitfor locked 30000", "wait 150", "press A 3", "waitfor link 20000", "wait 300", "winshot f/f05_trade_screen.png",
    "wait 120", "quit"])

# T6a: a Single Battle - A's first Pokemon set to 50 HP before; turn 1 A uses its 2nd move, B its 1st; A then runs
# (gives up); after it, "state hp" in state.log must read as before (50, and the friend's as it was). Fixed waits:
# run with run_t6.sh (WJPC_TEST_WINDOW_NOBATTLE, so battle frames aren't drawn - seconds each on the test machine)
write('battle_A', ["wait 60", "warp 0 9 10 9", "waitfor free 20000", "state hp 0 50", "state hp 0", "state hp 1", "wait 900",
    "press RIGHT 3", "wait 30", "press A 3", "wait 150", "press DOWN 3", "wait 20", "press DOWN 3", "wait 20",
    "press A 3", "wait 90", "press A 3", "wait 90",
    "waitfor link 20000", "waitfor action 20000", "wait 120", "press A 3", "wait 120", "press RIGHT 3", "wait 60",
    "press A 3", "wait 1500"]
    + ["press RIGHT 3", "wait 30", "press DOWN 3", "wait 30", "press A 3", "wait 600"] * 3
    + ["waitfor free 20000", "wait 120", "state hp 0", "state hp 1", "winshot f/c12_field.png", "wait 60", "quit"])
write('battle_B', ["wait 60", "warp 0 9 11 9", "waitfor free 20000", "state hp 0",
    "waitfor locked 30000", "wait 150", "press A 3", "wait 60",
    "waitfor link 20000", "waitfor action 20000", "wait 120", "press A 3", "wait 120", "press A 3",
    "wait 1500", "press A 3", "wait 120", "press A 3",
    "waitfor free 20000", "wait 120", "state hp 0", "winshot f/d12_field.png", "wait 400000", "quit"])

# T6b: Team Up - on Route 102, A at 25,14 above Bug Catcher Rick (25,15; his "beaten" flag cleared), B at 24,14. A faces B,
# Team Up > B yes (partners); A talks to Rick, B gets "join the battle?" > yes: the team-up battle; A sets both foes to
# 1 HP, both leads use Swift (both foes, never misses: the targets shown, A) - won in one turn; then each game's Exp. lines (A presses
# through them). state.log: levels and Exp. before and after, Rick's flag (1 on A only). Run with run_t6.sh.
tu_after = sum([["press A 3", "wait 100", "winshot f/%s%02d.png" % ("{}", k)] for k in range(16)], [])
tu_after_b = [x.replace("press A", "press B") for x in tu_after]   # (B: B Button - "no" to the four-move question)
write('teamup_A', ["wait 60", "warp 0 17 25 14", "waitfor free 20000", "state trainer 615 0", "state setmove 0 129",
    "partylevel 0 52",
    "state level 0", "state exp 0", "state level 1", "state exp 1", "wait 900", "winshot f/t00_together.png",
    "press LEFT 3", "wait 30", "press A 3", "wait 150", "winshot f/t01_menu.png",
    "press DOWN 3", "wait 20", "press DOWN 3", "wait 20", "press DOWN 3", "wait 20", "winshot f/t02_on_teamup.png",
    "press A 3", "wait 90", "winshot f/t03_asking.png", "wait 1200", "winshot f/t04_partners.png",
    "press A 3", "wait 90", "press A 3", "wait 90", "waitfor free 3000", "wait 60",
    "press DOWN 3", "wait 30", "press A 3", "wait 240", "winshot f/t05_rick_intro.png", "press A 3", "wait 120",
    "press A 3", "wait 90",
    "winshot f/t06_calling.png", "waitfor link 30000", "waitfor action 30000", "battlerhp 1 1", "battlerhp 3 1",
    "wait 60", "winshot f/t07_battle.png", "press A 3", "wait 120", "press A 3", "wait 120", "winshot f/t07b_targets.png",
    "press A 3"] + sum([["wait 60", "winshot f/t2%02d.png" % k] for k in range(30)], []) + [
    "waitfor nolink 60000", "wait 600", "winshot f/t08_back.png"]
    + [x.format("t1") for x in tu_after]
    + ["state level 0", "state exp 0", "state level 1", "state exp 1", "state trainer 615", "wait 60", "quit"])
write('teamup_B', ["wait 60", "warp 0 17 24 14", "waitfor free 20000", "state setmove 0 129", "partylevel 0 52",
    "state trainer 615 0",
    "state level 0", "state exp 0", "state trainer 615",
    "waitfor locked 30000", "wait 150", "winshot f/u01_teamup_q.png", "press A 3", "wait 90", "press A 3", "wait 90",
    "winshot f/u02_teamup_yesno.png", "press A 3", "wait 240", "winshot f/u03_partners.png", "press A 3", "wait 90",
    "press A 3", "wait 90", "waitfor free 3000",
    "waitfor locked 30000", "wait 150", "winshot f/u04_join_q.png", "press A 3", "wait 90", "winshot f/u05_join_yesno.png",
    "press A 3", "waitfor link 30000", "waitfor action 30000", "wait 30", "winshot f/u06_battle.png",
    "press A 3", "wait 120", "press A 3", "wait 120", "press A 3"] + sum([["wait 60", "winshot f/u2%02d.png" % k] for k in range(30)], []) + [
    "waitfor nolink 60000", "wait 600", "winshot f/u07_back.png"]
    + [x.format("u1") for x in tu_after_b]
    + ["state level 0", "state exp 0", "state trainer 615", "wait 400000", "quit"])

# T6b, a loss: as teamup_A/B, but everyone's Pokemon but the lead have 0 HP and both leads use Memento (the user faints)
# - the trainer wins; both pay as for a normal loss ("You gave ... to the winner") and black out.
# state.log: money before and after (both less), Rick's flag (0 on both).
tl_turns = ["press A 3", "wait 120", "press A 3", "wait 120", "press A 3", "wait 400"] * 6
tl_start_a = ["wait 60", "warp 0 17 25 14", "waitfor free 20000", "state trainer 615 0", "state setmove 0 262"] + \
    ["state hp %d 0" % i for i in range(1, 6)] + ["state money 0", "wait 900",
    "press LEFT 3", "wait 30", "press A 3", "wait 150", "press DOWN 3", "wait 20", "press DOWN 3", "wait 20", "press DOWN 3",
    "wait 20", "press A 3", "wait 1290", "press A 3", "wait 90", "press A 3", "wait 90", "waitfor free 3000", "wait 60",
    "press DOWN 3", "wait 30", "press A 3", "wait 240", "press A 3", "wait 120", "press A 3", "wait 90",
    "waitfor link 30000", "waitfor action 30000", "wait 60"]
write('teamlose_A', tl_start_a + tl_turns + sum([["wait 60", "winshot f/l2%02d.png" % k] for k in range(12)], []) + [
    "waitfor nolink 60000", "wait 900", "winshot f/l08_back.png"]
    + [x.format("l1") for x in tu_after]
    + ["state money 0", "state trainer 615", "state hp 0", "wait 60", "quit"])
write('teamlose_B', ["wait 60", "warp 0 17 24 14", "waitfor free 20000", "state setmove 0 262", "state trainer 615 0"]
    + ["state hp %d 0" % i for i in range(1, 6)] + ["state money 0",
    "waitfor locked 30000", "wait 150", "press A 3", "wait 90", "press A 3", "wait 90", "press A 3", "wait 240",
    "press A 3", "wait 90", "press A 3", "wait 90", "waitfor free 3000",
    "waitfor locked 30000", "wait 150", "press A 3", "wait 90", "press A 3", "waitfor link 30000", "waitfor action 30000",
    "wait 30"] + tl_turns + sum([["wait 60", "winshot f/m2%02d.png" % k] for k in range(12)], []) + [
    "waitfor nolink 60000", "wait 900", "winshot f/m08_back.png"]
    + [x.format("m1") for x in tu_after]
    + ["state money 0", "state trainer 615", "state hp 0", "wait 400000", "quit"])
