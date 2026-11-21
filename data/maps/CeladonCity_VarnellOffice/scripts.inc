@ Project ARIA ch. 5 — Varnell's office (scenes in data/scripts/aria_ch5.inc)
CeladonCity_VarnellOffice_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CeladonCity_VarnellOffice_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, Aria_Ch5_OfficeOnFrame
	.byte 0

CeladonCity_VarnellOffice_OnTransition::
	call Aria_Ch5_OfficeOnTransition
	end
