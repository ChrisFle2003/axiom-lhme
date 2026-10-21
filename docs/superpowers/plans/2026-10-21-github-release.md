# BRUCE LHME GitHub Release Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Turn the existing BRUCE LHME source directory into a public, MIT-licensed GitHub-ready C++20 repository with reproducible CI.

**Architecture:** FormulaRecipe stays the documented value source; KEVIN stays an explicit 8x8 domain/projection layer. Git metadata and GitHub assets surround the existing library without altering its runtime architecture. CMake presets remain the build interface locally and in CI.

**Tech Stack:** Git, GitHub Actions, CMake 3.25, C++20, CTest, YAML, Markdown.

**Spec:** `docs/superpowers/specs/2026-10-21-github-release-design.md`

## Global Constraints

- Repository: public `ChrisFle2003/bruce-lhme`; primary branch `main`.
- License: MIT, copyright Christopher Flegel.
- Preserve FormulaRecipe-first architecture; KEVIN is domain/projection only.
- CI runs Debug and Release on Ubuntu and Windows; sanitizers run on Linux only.
- Do not represent GPU, NanoCell runtime, semantic routing, AnswerPoints or Experience Records as shipped v0.1 functionality.
- Do not commit build directories, temporary snapshots, profiler output or IDE state.

## Review Focus

- Windows configuration: each CMake preset must work with a multi-config generator and select the requested configuration.
- Static-library instrumentation: an installed external consumer must receive sanitizer runtime link flags.
- Public README claims: only verified v0.1 behaviour is presented as implemented.
- Ignore rules: generated `build*` and `out/` paths are ignored while source and documentation remain trackable.
- CI trigger: pushes and pull requests both run the same Debug/Release matrix.

---

### Task 1: Establish the public repository foundation

**Files:**
- Create: `.gitignore`, `LICENSE`, `README.md`, local `.git` directory
- Modify: `docs/release-v0.1.md`

**Interfaces:**
- Consumes: current source tree and `docs/architecture.md`.
- Produces: a public project landing page and Git repository on branch `main`.

- [ ] **Step 1: Initialize Git on the `main` branch and inspect the initial tracked set**

Run: `git init -b main && git status --short`

Expected: local repository initialized; project files appear as untracked before ignore rules are added.

- [ ] **Step 2: Add public repository files**

Create `.gitignore` for CMake outputs (`build*/`, `out/`, `CMakeFiles/`, `CMakeCache.txt`, `Testing/`), generated binaries, IDE state and temporary snapshots. Create exact MIT license text for Christopher Flegel. Create a README containing purpose, FormulaRecipe/KEVIN boundary, requirements, Debug/Release builds, package-consumer snippet, test command and technical-document links. Add the repository URL to the release document once the remote exists.

- [ ] **Step 3: Verify ignore and public-document contracts**

Run: `git check-ignore -v out/build/debug/CMakeCache.txt && git status --short && grep -q "MIT License" LICENSE && grep -q "FormulaRecipe" README.md`

Expected: generated build cache is ignored; README and LICENSE contain the promised public contract.

- [ ] **Step 4: Commit the repository foundation**

Run: `git add .gitignore LICENSE README.md docs/release-v0.1.md && git commit -m "chore: establish public repository foundation"`

### Task 2: Make CMake presets portable and add GitHub automation

**Files:**
- Modify: `CMakePresets.json`
- Create: `tests/cmake_presets_contract_test.cmake`
- Create: `.github/workflows/ci.yml`, `.github/ISSUE_TEMPLATE/bug_report.yml`, `.github/pull_request_template.md`

**Interfaces:**
- Consumes: preset names `debug`, `release`, `address`, `undefined`, `thread`.
- Produces: portable CMake presets and CI invocable by `cmake --preset <name>`, `cmake --build --preset <name>`, `ctest --preset <name>`.

- [ ] **Step 1: Write a failing CMake preset validation for Windows multi-config semantics**

Add `tests/cmake_presets_contract_test.cmake`. It reads `CMakePresets.json`, requires that the `base` preset omits a Unix-only generator, and requires Debug/Release build and test presets to select `Debug`/`Release` explicitly.

- [ ] **Step 2: Run the new validation before modifying presets**

Run: `cmake -P tests/cmake_presets_contract_test.cmake`

Expected: FAIL because `base` fixes `Unix Makefiles` and build/test presets lack explicit configurations.

- [ ] **Step 3: Implement portable preset configuration and GitHub files**

Remove the fixed `Unix Makefiles` generator from the base preset. Add `configuration` fields to build/test presets (`Debug` for debug/address/undefined/thread, `Release` for release) while retaining `CMAKE_BUILD_TYPE` for single-config generators. Add Actions: Ubuntu/Windows Debug+Release matrix and Linux Address+Undefined sanitizer matrix. Add templates requesting reproduction details, platform/compiler, tests, documentation and FormulaRecipe/KEVIN boundary review.

- [ ] **Step 4: Verify CI inputs and CMake contract**

Run: `cmake -P tests/cmake_presets_contract_test.cmake && cmake --list-presets && cmake --preset debug && cmake --build --preset debug -j2 && ctest --preset debug`

Expected: contract passes and the Debug profile completes with all CTest tests passing.

- [ ] **Step 5: Commit automation assets**

Run: `git add CMakePresets.json tests/cmake_presets_contract_test.cmake .github && git commit -m "ci: add cross-platform GitHub verification"`

### Task 3: Create the release-candidate history and connect GitHub

**Files:**
- Modify: `docs/release-v0.1.md`
- Add: all remaining source, tests, examples, concepts and documentation files to initial history

**Interfaces:**
- Consumes: Tasks 1-2.
- Produces: clean `main` history ready to push to `https://github.com/ChrisFle2003/bruce-lhme.git`.

- [ ] **Step 1: Stage and inspect the first source release**

Run: `git add CMakeLists.txt CMakePresets.json cmake concepts demos docs examples include src tests && git diff --cached --stat && git status --short`

Expected: source, tests, public documents and automation are staged; no generated output is staged.

- [ ] **Step 2: Run the release verification suite**

Run: `cmake --preset release && cmake --build --preset release -j2 && ctest --preset release`

Expected: all CTest tests pass, including package layout and external-consumer checks.

- [ ] **Step 3: Commit the release candidate**

Run: `git commit -m "feat: publish BRUCE LHME v0.1 release candidate" && git log --oneline --decorate -3`

Expected: `main` contains the foundation, CI assets and release-candidate source commit(s).

- [ ] **Step 4: Create or connect the GitHub remote and push**

Create an empty public `ChrisFle2003/bruce-lhme` repository without generated files, then run `git remote add origin https://github.com/ChrisFle2003/bruce-lhme.git && git push -u origin main`.

Expected: GitHub displays the README, MIT license and initial Actions workflow. If repository creation is unavailable through the connected GitHub capability, stop after local commits and provide the exact one-click GitHub creation step rather than inventing a remote URL.

## Self-review

- Spec coverage: Tasks 1-3 cover public identity, MIT license, documentation, ignore policy, Actions, contribution templates, `main` history and remote publication.
- Review focus: Task 2 owns portable presets; Task 1 owns public claims and ignore rules; Task 3 owns external-consumer verification and safe push.
- Consistency: all task references use the existing `debug`, `release`, `address`, `undefined` and `thread` preset names.
- Proportion: the plan groups the work into three independently verifiable deliverables.
