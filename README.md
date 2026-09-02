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
```

Each execution validates all selected model artifacts and creates a unique
run directory containing the full result, provenance, structured log, and a
concise summary. See the User Guide for interpretation limits; this workflow
is a reproducible software demonstrator, not a clinically validated assay.

Use `example copy --id <example-id> --output <directory>` to create a safe,
licensed starter configuration without editing the checked-in reference.

MEHLISSA Next is a research model. It is not a medical device and does not
provide patient-specific clinical predictions.
