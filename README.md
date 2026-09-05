<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# MEHLISSA

MEHLISSA Next is a research platform for reproducible multiscale simulation
of medical nanotechnology and molecular communication in the human body. The
repository also retains the historical MEHLISSA implementations as scientific
and technical references.

Start here:

- [Project Status and Collaboration Brief](docs/PROJECT_STATUS_AND_COLLABORATION_BRIEF.md)
  — current capabilities, evidence maturity, limitations, and high-value next
  contributions; a shareable PDF is linked from the document.
- [User Guide](docs/USER_GUIDE.md) — understand MEHLISSA, choose an experiment,
  build and run the software, and interpret model evidence and limitations.
- [Development Guide](docs/DEVELOPMENT.md) — compiler, CMake, vcpkg, CI, and
  contribution-oriented setup.
- [Software Architecture and Developer Guide](docs/architecture/SOFTWARE_ARCHITECTURE.md)
  — system structure, public APIs, module extensions, and personalization.
- [Roadmap](docs/ROADMAP.md) — achieved M0-M7 and research-use delivery record,
  scientific qualification program, further scenarios, additional organs,
  scaling, and the governed digital-twin path.
- [Active pulmonary and capillary qualification protocol](docs/qualification/PULMONARY_CAPILLARY_QUALIFICATION_PROTOCOL.md)
  — frozen candidates, bounded claim, endpoints, data separation, uncertainty,
  controls, and the next evidence-acquisition increment.
- [Legacy-baseline analysis](docs/IST_ANALYSE.md) — dated inventory of the
  historical implementations at commit `4f4fc5a`; it is not the current Next
  status.

MEHLISSA provides native source-build paths for Windows/MSVC, Linux/GCC and
Linux/Clang, and macOS/Apple Clang. The macOS path builds the command-line
simulator and supports the same local Workbench workflow; it intentionally does
not create an application bundle, installer, signed binary, or downloadable CI
artifact. See the Development Guide for the platform-specific commands. The
ARM64 path is accepted by the complete Apple Clang build and test job in
[CI run 33956456353](https://github.com/RegineWendt/MEHLISSA/actions/runs/33956456353).

After building the normal Debug preset, the complete M7 fingerprinting
research demonstrator can be run without writing C++:

```powershell
build/windows-msvc/apps/Debug/mehlissa.exe model list
build/windows-msvc/apps/Debug/mehlissa.exe model describe `
  --id organ.pulmonary-zero-dimensional
build/windows-msvc/apps/Debug/mehlissa.exe example list `
  --model organ.pulmonary-zero-dimensional
build/windows-msvc/apps/Debug/mehlissa.exe scenario list
build/windows-msvc/apps/Debug/mehlissa.exe scenario run `
  --file examples/scenarios/fp9-lung-level-a-v1.json `
  --output results/fp9-reference
build/windows-msvc/apps/Debug/mehlissa.exe result report `
  --file results/fp9-reference/<run-directory>/result.json `
  --output reports/fp9-reference
build/windows-msvc/apps/Debug/mehlissa.exe campaign validate `
  --file examples/campaigns/fp9-collector-count-v1.json
build/windows-msvc/apps/Debug/mehlissa.exe campaign run `
  --file examples/campaigns/fp9-collector-count-v1.json `
  --output results/fp9-collector-campaign
```

Each execution validates all selected model artifacts and creates a unique
run directory containing the full result, provenance, structured log, and a
concise summary. See the User Guide for interpretation limits; this workflow
is a reproducible software demonstrator, not a clinically validated assay.

Use `example copy --id <example-id> --output <directory>` to create a safe,
licensed starter configuration without editing the checked-in reference.
Use `result report --file <result.json> --output <directory>` to create a
shareable HTML report, stable CSV tables, a concise text summary, and a bundled
copy of the complete machine-readable result.
Use `campaign validate --file <campaign.json>` before `campaign run --file
<campaign.json> --output <new-directory>` to create controlled replicates,
collector-count sweeps, and paired comparisons. The campaign bundle preserves
every derived scenario and result and adds aggregate JSON and CSV indexes.

Python users can install the lightweight UX-5 helpers from the repository or
use them directly through `PYTHONPATH=python`. The Python layer invokes the same
C++ executable and reads its versioned JSON results; it does not implement a
second simulator:

```python
from mehlissa import MehlissaClient, load_result

client = MehlissaClient("build/windows-msvc/apps/Debug/mehlissa.exe", ".")
execution = client.run_scenario(
    "examples/scenarios/fp9-lung-level-a-v1.json", "results/python-run"
)
print(load_result(execution.result).summary)
```

See `examples/notebooks/01-first-scenario.ipynb` and
`examples/notebooks/02-campaign-analysis.ipynb` for complete workflows.

The completed UX-6 graphical workbench is released as **MEHLISSA Research
Workbench 1.0.0**. It browses the same validated model and example catalog and
creates a schema-guided derivative of the complete FP9/lung scenario in a
protected local page. A virtual-environment installation provides the stable
console entry point:

```powershell
py -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install .
mehlissa-workbench --repository-root . --check
mehlissa-workbench --repository-root .
```

The launcher locates a normal build automatically or accepts `--executable`.
Guided edits receive live field/document diagnostics, stable error codes,
repair guidance, warning/error separation, a copyable validation summary, and
authoritative non-overwriting save-as inside a repository-local workspace.
After explicit plan confirmation, it runs either the exact validated candidate
or the curated six-run replicate/sweep/paired-comparison campaign through the
same Python process API. Unique bounded output, progress, cancellation, logs,
and retained artifact views remain traceable. See the User Guide for the
complete workflow and diagnostics. Completed scenarios and campaigns can then
be read as outcome, stage, case, group, and paired-difference dashboards through
the accepted result readers. Two completed scenarios can be compared; failed,
cancelled, partial, and missing results are explicitly excluded.
Every completed view also carries an artifact-backed provenance audit with
software/build identity, seeds, input/model/schema/result hashes, evidence and
licence declarations, maturity labels, limitations, and a persistent
non-clinical boundary. The complete audit can be downloaded as JSON; altered
hashes and incomplete metadata are flagged rather than repaired or inferred.
Completed campaigns additionally expose accepted-reader replicate observations,
collector-count sweeps, and same-seed paired differences as unit-labelled,
sample-counted accessible figures and exact-value tables. Source-bound JSON,
CSV, and SVG downloads reproduce the displayed values. Observed ranges are
descriptive only and are explicitly not confidence intervals or clinical
uncertainty estimates.
The versioned Python wheel contains the process API, readers, local host, and
browser assets. The matching built C++ executable and repository-held models,
schemas, evidence, and examples remain explicit prerequisites rather than being
silently duplicated. See the [example workspace](examples/workbench/README.md)
and [UX-6.8 release acceptance](docs/ux/UX6_8_RELEASE_ACCEPTANCE.md).

MEHLISSA Next is a research model. It is not a medical device and does not
provide patient-specific clinical predictions.
