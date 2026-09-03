<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA Workbench example workspace

This small, repository-backed workspace is the recommended starting point for
the MEHLISSA Research Workbench 1.0. It uses the reviewed examples and schemas
already shipped in this repository; the workbench creates editable copies in
`workbench-scenarios/` and retained run evidence in `workbench-runs/` by
default. Both directories are created on demand and remain local.

The workbench is research software. It does not represent an identified
person, accept clinical records, or support diagnosis or treatment decisions.

## Start on Windows

From a Developer PowerShell in the repository root, after building the Debug
application and installing the Python package in a virtual environment:

```powershell
mehlissa-workbench --repository-root . --check
mehlissa-workbench --repository-root .
```

If automatic executable discovery does not find the build, add:

```powershell
--executable build/windows-msvc/apps/Debug/mehlissa.exe
```

## Start on Linux

From the repository root, after building and installing the Python package:

```bash
mehlissa-workbench --repository-root . --check
mehlissa-workbench --repository-root .
```

If needed, pass `--executable build/linux-gcc/apps/mehlissa`.

## Novice task: inspect and run one validated scenario

1. Open **Scenario workspace** and select **FP9 complete fingerprinting**.
2. Review the purpose, model inventory, maturity, sources, and non-clinical
   boundary before changing anything.
3. Change one guided field. Wait for the authoritative validation status and
   use the located repair message if the value is invalid.
4. Save the valid candidate under a new name. Existing examples are never
   overwritten.
5. Select **One scenario**, review the exact run plan and destination, confirm,
   and start it.
6. Open the completed result dashboard. Inspect the outcome, stages, retained
   JSON/CSV/report artifacts, evidence identities, and audit download.

## Expert task: inspect a controlled campaign

1. Select the allowlisted **FP9 collector-count sensitivity campaign**.
2. Review the six-run design: fixed-configuration replicates, collector-count
   sweep, and same-seed comparison.
3. Confirm and execute the campaign.
4. Open the result dashboard and switch among sensitivity, specificity,
   detected, and assembled responses.
5. Compare replicate variation with deterministic parameter contrasts. The
   displayed range is not a confidence interval and the small campaign is not
   population evidence.
6. Export the exact analysis JSON, CSV, and accessible SVG. Match their source
   SHA-256 to the retained campaign result before sharing them.

## Keyboard and assistive-technology route

Use `Tab` and `Shift+Tab` to traverse controls, `Enter` or `Space` to activate
them, and `Escape` to close dialogs. The skip link moves directly to the main
workspace. Validation, run state, and result loading are announced as status or
alert messages. Tables remain usable by horizontal scrolling at narrow widths;
the page itself reflows without global horizontal scrolling.

## Add a model or scenario

Do not add scientific behavior to the browser. Implement and register the model
in the C++ model layer, publish its versioned metadata and evidence, expose it
through the accepted CLI/process API, add schemas and validation, then add a
reviewed example. The workbench catalog discovers the accepted interface.
Detailed steps are in the
[software architecture guide](../../docs/architecture/SOFTWARE_ARCHITECTURE.md).
