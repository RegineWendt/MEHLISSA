# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Command-line entry point for the local UX-6.1 prototype."""

from __future__ import annotations

import argparse
from pathlib import Path
import threading
import webbrowser

from mehlissa import MehlissaClient

from .server import LOOPBACK_HOSTS, create_server, discover_catalog


def _default_executable(repository_root: Path) -> Path | None:
    candidates = (
        "build/windows-msvc/apps/Release/mehlissa.exe",
        "build/windows-msvc/apps/Debug/mehlissa.exe",
        "build/linux-gcc/apps/mehlissa",
        "build/linux-clang-analysis/apps/mehlissa",
    )
    return next(
        (repository_root / candidate for candidate in candidates if (repository_root / candidate).is_file()),
        None,
    )


def _parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="mehlissa-workbench",
        description="Start the loopback-only read-only MEHLISSA UX-6.1 prototype.",
    )
    parser.add_argument("--repository-root", type=Path, default=Path.cwd())
    parser.add_argument("--executable", type=Path)
    parser.add_argument("--host", default="127.0.0.1", choices=sorted(LOOPBACK_HOSTS))
    parser.add_argument("--port", type=int, default=8765)
    parser.add_argument("--no-browser", action="store_true")
    parser.add_argument(
        "--check",
        action="store_true",
        help="verify read-only discovery and exit without starting a server",
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
        print("workbench_status=ready")
        print("read_only=true")
        print(f"model_count={len(catalog['models'])}")
        print(f"example_count={len(catalog['examples'])}")
        return 0

    server = create_server(client, arguments.host, arguments.port)
    print("MEHLISSA Next Research Workbench — UX-6.1 foundation prototype")
    print("Read-only: this prototype cannot create, change, or run a scenario.")
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
