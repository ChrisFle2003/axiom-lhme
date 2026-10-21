# BRUCE LHME v0.1 — GitHub Release Design

## Purpose

Publish the existing BRUCE Lazy Hierarchical Memory Engine as a public,
reusable C++20 library. The GitHub repository shall make it easy for another
developer to understand the FormulaRecipe-first model, build it locally, run
the supported tests and contribute safely.

The designated GitHub owner is `ChrisFle2003` (Christopher Flegel / Duzi). The
repository name is `bruce-lhme`.

## Public contract

The repository is public and licensed under MIT. Its top-level documentation
will state the current v0.1 scope plainly:

```text
FormulaRecipe + FormulaAddress -> FormulaOverrideStore -> Engine
KEVIN GridDomain + explicit Projection -> FormulaEvaluator
```

FormulaRecipe remains the value source. KEVIN is a hierarchical 8x8 domain,
filter and dimension coordinate; it never implicitly chooses a value formula.
GPU, NanoCell runtime, semantic routing, AnswerPoints and Experience Records
remain out of scope for v0.1.

## Repository contents

- `README.md`: purpose, architecture, fast build, package-consumer example,
  stable status and links to the technical release contract.
- `LICENSE`: the exact MIT license text, copyright held by Christopher Flegel.
- `.gitignore`: CMake outputs, test artifacts, IDE files and local profiling
  artifacts; source, examples, concepts and documentation remain tracked.
- `.github/workflows/ci.yml`: GitHub Actions checks on Ubuntu and Windows,
  executing the Debug and Release CMake presets. Sanitizer jobs stay Linux-only.
- `.github/ISSUE_TEMPLATE/bug_report.yml`: asks for expected behaviour,
  reproduction steps, platform/compiler and BRUCE trace where applicable.
- `.github/pull_request_template.md`: requests tests, documentation and
  FormulaRecipe/KEVIN boundary review.

## Source-control and publication flow

1. Initialize the local directory as a Git repository on `main`.
2. Add the GitHub-facing files plus the existing source, tests, examples and
   documentation in an initial commit named `Initial BRUCE LHME v0.1 release
   candidate`.
3. Create the public `ChrisFle2003/bruce-lhme` repository without a generated
   README, licence or `.gitignore`, then add it as `origin`.
4. Push `main` and let GitHub Actions run.
5. Create the first public v0.1 tag only after its platform checks and release
   checklist are complete.

The current GitHub connector exposes account and repository management, but
does not expose a repository-creation operation. If it remains unavailable at
publication time, creating the empty repository in GitHub's web UI is the only
manual action; the prepared local repository then pushes directly to it.

## Verification

- Validate the repository files against the documented scope.
- Build and run the full CTest suite through the Debug and Release presets.
- Verify the installed-package external consumer through CTest.
- After publication, verify that GitHub Actions receives the initial push and
  reports the platform jobs.

## Decisions

- Repository visibility: public.
- Licence: MIT.
- Owner: `ChrisFle2003`.
- Primary branch: `main`.
