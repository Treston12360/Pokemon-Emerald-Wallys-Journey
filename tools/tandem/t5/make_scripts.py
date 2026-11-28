#!/usr/bin/env python3
"""Project Tandem T5 test scripts (WJPC_SCRIPT) for two games, A (asks) and B (answers).

python3 make_scripts.py FASTBOOT.txt OUTDIR   ->  trade_A/B.txt, edge_A/B.txt, leave_A/B.txt
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
