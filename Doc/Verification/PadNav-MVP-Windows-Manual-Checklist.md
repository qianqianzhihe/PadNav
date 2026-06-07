# PadNav MVP Windows 11 Manual Checklist

## Execution Context

- Date: `2026-06-07`
- Workspace: `E:\programCache\CodeProject\QtCode\PadNav`
- Target app: `padnav`
- OS target: `Windows 11`
- Browser target: `Chrome`
- Controller target: `Controller (BEITONG A1S2 XINPUT GAMEPAD)`

## Automated Verification

Executed in this session:

```powershell
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug --target padnav_tests padnav -- -j1
ctest --test-dir cmake-build-debug -R padnav_tests --output-on-failure
```

Results:

- [x] PASS: `cmake -S . -B cmake-build-debug`
- [x] PASS: `cmake --build cmake-build-debug --target padnav_tests padnav -- -j1`
- [x] PASS: `ctest --test-dir cmake-build-debug -R padnav_tests --output-on-failure`
- [x] PASS: Startup smoke in this session kept `padnav.exe` running for 2 seconds without early exit

## Manual Verification Record

Instruction:

- Record each item as `[x] PASS`
- Or record as `[ ] FAIL: <observed behavior>`

### Environment

- [ ] Windows 11
- [ ] Chrome
- [ ] Controller (BEITONG A1S2 XINPUT GAMEPAD)

### Chrome 15-minute session

- [ ] Pointer movement has no obvious stutter, drift, or accidental movement
- [ ] Scroll direction and continuity are correct
- [ ] Left click works
- [ ] Right click works
- [ ] Base layer shortcuts work
- [ ] Navigation layer shortcuts work
- [ ] Tab layer shortcuts work

### Tray behavior

- [ ] Startup shows tray only
- [ ] Settings close hides the window
- [ ] Pause blocks input injection while monitor still updates
- [ ] Resume restores input injection
- [ ] View button toggles pause and resume once per press
- [ ] Exit removes the tray icon and terminates the process

### Controller reconnect

- [ ] Disconnect keeps the app alive
- [ ] Reconnect restores input automatically

### Configuration

- [ ] Apply writes `%APPDATA%/PadNav/config.json`
- [ ] Restart restores saved values
- [ ] Cancel discards unapplied changes
- [ ] Missing JSON silently restores defaults
- [ ] Malformed JSON silently restores defaults

### Input Monitor

- [ ] Open Input Monitor from Settings
- [ ] Shows connection state
- [ ] Shows stick positions
- [ ] Shows button state
- [ ] Shows active layer
- [ ] Shows mapping enabled state
- [ ] Closing Input Monitor does not stop mapping

### Crash Dumps

- [ ] Forced unhandled exception writes `MiniDumpNormal` under `%LOCALAPPDATA%/PadNav/CrashDumps/`
- [ ] Startup leaves at most 3 dump files

## Notes

- Manual verification is still required for all unchecked items above.
- This file intentionally records only results actually verified in the current session.
