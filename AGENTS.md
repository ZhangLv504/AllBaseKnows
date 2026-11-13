# Workspace Instructions

## Required startup context

Before answering or acting on any learning request in this workspace, read these files completely:

1. `Doc/需求.md` — the authoritative teaching workflow, routing rules, note format, and completion rules.
2. `Doc/学习记录.md` — the authoritative source for the current topic, current knowledge point, completion state, unresolved issues, and next step.

Do not assume progress from a previous chat. Restore it from the files above. If these files conflict with an older conversation, follow the current workspace files and the user's latest direct instruction.

## Topic ownership

- The user chooses every learning topic.
- Do not invent, preselect, or automatically begin a new topic.
- Advance only one complete knowledge point at a time.
- Do not mark a knowledge point as passed until the user explicitly confirms mastery or the submitted exercise has been inspected and verified.

## Stable routing

| Topic | Exercise project | Knowledge notes |
| --- | --- | --- |
| C++ | `CppKnowles` | `Doc/Cpp知识点.md` |
| Qt | `QtKnowles` | `Doc/Qt知识点.md` |
| Python | `PythonKnows` | `Doc/Python知识点.md` |
| Java | `JavaKnows/StudyProj` | `Doc/Java知识点.md` |
| CMake 构建 | 复用现有工程的 `CMakeLists.txt`（如 `QtKnowles`） | `Doc/CMake知识点.md` |

For other topics, follow `Doc/需求.md`. If no existing project or note file clearly matches, ask the user where the topic should be stored before creating anything.

## Documentation ownership

- Keep the full workflow details in `Doc/需求.md`; do not duplicate them here.
- Keep live progress and verification results in `Doc/学习记录.md`.
- Keep complete teaching content in the routed knowledge-note file.
- Do not put “通关检查清单” or “通关验证” sections in a knowledge-note file.
- Update `Doc/学习记录.md` at the end of each teaching round as required by `Doc/需求.md`.

## Project boundary

- `CppKnowles` is the standalone C++17 project and must not acquire a Qt dependency unless the user explicitly requests it.
- `QtKnowles` is the Qt project. Use Qt 5.15.2 by default; do not upgrade, downgrade, or switch major versions unless the user explicitly requests it.
- Preserve unrelated user changes and validate exercises using the routed project.
