# PadNav Agent Notes

## Code Editing Skills

- For Qt/C++ code editing, load and follow `chenml-coding-editor` first.
- Then load and apply `cpp-coding-standards` as the secondary C++ safety and style baseline.
- If a task is outside `chenml-coding-editor` scope, state that briefly and continue with the best matching project skill.

## File Naming

- C++ source and header filenames must use lowercase compact names without underscores.
- Prefer Qt Creator style names such as `xinputcontroller.h` / `xinputcontroller.cpp`.
- Do not introduce new `.h` or `.cpp` filenames like `xinput_controller.h`, `mapping_engine.cpp`, or `profile_tests.cpp`; use `xinputcontroller.h`, `mappingengine.cpp`, and `profiletests.cpp` instead.
- When renaming existing files, update includes, CMake source lists, and tests in the same change.
