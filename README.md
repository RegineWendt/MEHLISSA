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

- [User Guide](docs/USER_GUIDE.md) — understand MEHLISSA, choose an experiment,
  build and run the software, and interpret model evidence and limitations.
- [Development Guide](docs/DEVELOPMENT.md) — compiler, CMake, vcpkg, CI, and
  contribution-oriented setup.
- [Software Architecture and Developer Guide](docs/architecture/SOFTWARE_ARCHITECTURE.md)
  — system structure, public APIs, module extensions, and personalization.
- [Roadmap](docs/ROADMAP.md) — implementation strategy from the validated body
  layer to organ, capillary, cellular, and Nano-IoT models.
- [Current-State Analysis](docs/IST_ANALYSE.md) — legacy inventory and the gap
  between existing software and the dissertation vision.

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

The UX-6.1 graphical foundation can browse the same validated model and example
catalog in a protected, read-only local page:

```powershell
$env:PYTHONPATH = "$PWD/python"
python -m mehlissa_workbench --repository-root . --check
python -m mehlissa_workbench --repository-root .
```

The launcher locates a normal build automatically or accepts `--executable`.
Scenario editing and execution remain command/Python workflows until later
UX-6 increments. See the User Guide for the workbench boundary and diagnostics.

MEHLISSA Next is a research model. It is not a medical device and does not
provide patient-specific clinical predictions.
