# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Command-line entry point for the local MEHLISSA research workbench."""

from __future__ import annotations

import argparse
from pathlib import Path
import threading
import webbrowser

from mehlissa import MehlissaClient

from .server import (
    LOOPBACK_HOSTS,
    RunWorkspace,
    ScenarioWorkspace,
    create_server,
    discover_catalog,
)


def _default_executable(repository_root: Path) -> Path | None:
    candidates = (
        "build/windows-msvc/apps/Release/mehlissa.exe",
        "build/windows-msvc/apps/Debug/mehlissa.exe",
        "build/linux-gcc/apps/mehlissa",
        "build/linux-clang-analysis/apps/mehlissa",
    )
    return next(
        (
            repository_root / candidate
            for candidate in candidates
            if (repository_root / candidate).is_file()
        ),
        None,
    )


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="mehlissa-workbench",
        description="Start the loopback-only MEHLISSA research workbench.",
    )
    parser.add_argument("--repository-root", type=Path, default=Path.cwd())
    parser.add_argument("--executable", type=Path)
    parser.add_argument("--host", default="127.0.0.1", choices=sorted(LOOPBACK_HOSTS))
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument(
        "--workspace",
        type=Path,
        help="scenario save-as directory inside the repository (default: workbench-scenarios)",
    )
    parser.add_argument(
        "--runs",
        type=Path,
        help="unique run-evidence directories inside the repository (default: workbench-runs)",
    )
    parser.add_argument("--no-browser", action="store_true")
    parser.add_argument(
        "--check",
        action="store_true",
        help="verify discovery, workspace loading, and starter validation, then exit",
    )
    return parser


def main() -> int:
    parser = _parser()
    arguments = parser.parse_args()
    repository_root = arguments.repository_root.expanduser().resolve()
    executable = arguments.executable
    if executable is None:
        executable = _default_executable(repository_root)
    if executable is None:
        parser.error("MEHLISSA executable not found; pass --executable <path>")

    client = MehlissaClient(executable, repository_root)
    if arguments.check:
        catalog = discover_catalog(client)
        scenario_workspace = ScenarioWorkspace(client, arguments.workspace)
        workspace = scenario_workspace.overview()
        source_id = str(workspace["sources"][0]["id"])
        validation = scenario_workspace.validate(source_id, {})
        print("workbench_status=ready")
        print("scenario_editing=true")
        print(f"model_count={len(catalog['models'])}")
        print(f"example_count={len(catalog['examples'])}")
        print(f"scenario_source_count={len(workspace['sources'])}")
        print(f"scenario_validation={'valid' if validation['valid'] else 'invalid'}")
        run_workspace = RunWorkspace(client, scenario_workspace, arguments.runs)
        print(f"run_plan_count={len(run_workspace.overview()['campaigns'])}")
        return 0

    server = create_server(
        client, arguments.host, arguments.port,
        workspace_root=arguments.workspace, runs_root=arguments.runs,
    )
    print("MEHLISSA Next Research Workbench — UX-6.5 result dashboard and comparison")
    print("Validated scenarios and the curated six-run campaign start after explicit confirmation.")
    print(f"workbench_url={server.url}")
    print("Press Ctrl+C to stop.")
    if not arguments.no_browser:
        opener = threading.Timer(0.25, webbrowser.open, args=(server.url,))
        opener.daemon = True
        opener.start()
    try:
        server.serve_forever(poll_interval=0.2)
    except KeyboardInterrupt:
        print("\nWorkbench stopped.")
    finally:
        server.server_close()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
