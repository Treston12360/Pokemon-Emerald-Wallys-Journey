Widescreen test pictures (PC port plan, step 2). Not part of any build.
capture.py: builds the GBA test ROM (tools/wj_testfw) once per camera offset (dx -8..8, dy -4..4 tiles),
  boots each scene with the mGBA harness and saves a screenshot. Env: WJ_REPO (repo root), WS_DIR (output dir,
  must contain s.txt = boot script), WJ_HARNESS (path to the harness built from tools/wj_testfw/harness.c).
  Scene i is picked by the RTC day: the epochs in the script give scene i in summer at noon.
  For clear skies, temporarily add `if (1) return ...; // WSMOCK` at the top of WallyJourney_TryRegionalCoordWeather,
  WallyJourney_UpdateLiveWeather and WallyJourney_SeasonalWeather, then `git checkout src/wally_journey.c`.
compose.py: stitches the shots into one wider view (player sprite area masked in the off-centre shots).
