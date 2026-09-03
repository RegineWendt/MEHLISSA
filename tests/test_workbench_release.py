# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""UX-6.8 clean-install, packaging, and static-accessibility acceptance."""

from __future__ import annotations

import argparse
from collections import Counter
from html.parser import HTMLParser
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
import venv
import zipfile


PARSER = argparse.ArgumentParser()
PARSER.add_argument("--executable", required=True)
PARSER.add_argument("--root", required=True)
ARGS = PARSER.parse_args()


class WorkbenchMarkup(HTMLParser):
    """Collect release-critical semantic properties without a browser dependency."""

    def __init__(self) -> None:
        super().__init__()
        self.ids: list[str] = []
        self.html_language: str | None = None
        self.label_depth = 0
        self.label_targets: set[str] = set()
        self.controls: list[tuple[str, str | None, bool, bool]] = []
        self.buttons_without_type: list[str | None] = []
        self.dialog_labels: list[str | None] = []
        self.fragment_targets: list[str] = []
        self.positive_tabindex: list[str | None] = []
        self.live_regions = 0
        self.h1_count = 0

    def handle_starttag(self, tag: str, attributes: list[tuple[str, str | None]]) -> None:
        values = dict(attributes)
        identifier = values.get("id")
        if identifier:
            self.ids.append(identifier)
        if tag == "html":
            self.html_language = values.get("lang")
        if tag == "label":
            self.label_depth += 1
            if values.get("for"):
                self.label_targets.add(str(values["for"]))
        if tag in {"input", "select", "textarea"} and values.get("type") != "hidden":
            has_aria_name = bool(values.get("aria-label") or values.get("aria-labelledby"))
            self.controls.append((tag, identifier, self.label_depth > 0, has_aria_name))
        if tag == "button" and not values.get("type"):
            self.buttons_without_type.append(identifier)
        if tag == "dialog":
            self.dialog_labels.append(values.get("aria-labelledby"))
        if tag == "a" and str(values.get("href", "")).startswith("#"):
            self.fragment_targets.append(str(values["href"])[1:])
        if values.get("tabindex") and int(str(values["tabindex"])) > 0:
            self.positive_tabindex.append(identifier)
        if values.get("aria-live"):
            self.live_regions += 1
        if tag == "h1":
            self.h1_count += 1

    def handle_endtag(self, tag: str) -> None:
        if tag == "label":
            self.label_depth -= 1


def run_checked(command: list[str], *, cwd: Path, environment: dict[str, str]) -> str:
    completed = subprocess.run(
        command,
        cwd=cwd,
        env=environment,
        check=False,
        capture_output=True,
        text=True,
        timeout=180,
    )
    if completed.returncode != 0:
        raise AssertionError(
            f"command failed ({completed.returncode}): {' '.join(command)}\n"
            f"stdout:\n{completed.stdout}\nstderr:\n{completed.stderr}"
        )
    return completed.stdout


class WorkbenchReleaseAcceptance(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = Path(ARGS.root).resolve()
        cls.executable = Path(ARGS.executable).resolve()

    def test_semantic_shell_has_release_accessibility_invariants(self) -> None:
        source = (
            self.root / "python" / "mehlissa_workbench" / "static" / "index.html"
        ).read_text(encoding="utf-8")
        markup = WorkbenchMarkup()
        markup.feed(source)
        duplicates = sorted(
            identifier for identifier, count in Counter(markup.ids).items() if count > 1
        )
        unnamed = [
            identifier or tag
            for tag, identifier, wrapped, has_aria_name in markup.controls
            if not wrapped and not has_aria_name and identifier not in markup.label_targets
        ]
        self.assertEqual(markup.html_language, "en")
        self.assertEqual(markup.h1_count, 1)
        self.assertEqual(duplicates, [])
        self.assertEqual(unnamed, [])
        self.assertEqual(markup.buttons_without_type, [])
        self.assertEqual(markup.positive_tabindex, [])
        self.assertGreaterEqual(markup.live_regions, 3)
        self.assertTrue(markup.dialog_labels)
        self.assertTrue(all(label in markup.ids for label in markup.dialog_labels))
        self.assertTrue(all(target in markup.ids for target in markup.fragment_targets))

        stylesheet = (
            self.root / "python" / "mehlissa_workbench" / "static" / "styles.css"
        ).read_text(encoding="utf-8")
        self.assertIn(":focus-visible", stylesheet)
        self.assertIn("@media (max-width: 460px)", stylesheet)
        self.assertNotIn("outline: none", stylesheet)

    def test_wheel_installs_into_clean_environment_and_runs_check(self) -> None:
        temporary_parent = self.root / "tmp"
        temporary_parent.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(
            prefix="ux6-8-release-", dir=temporary_parent
        ) as directory_text:
            directory = Path(directory_text)
            wheel_directory = directory / "wheel"
            wheel_directory.mkdir()
            package_source = directory / "source"
            package_source.mkdir()
            for filename in ("pyproject.toml", "README.md", "LICENSE.md"):
                shutil.copy2(self.root / filename, package_source / filename)
            shutil.copytree(
                self.root / "python",
                package_source / "python",
                ignore=shutil.ignore_patterns("*.egg-info", "__pycache__", "*.pyc"),
            )
            environment = os.environ.copy()
            environment.pop("PYTHONPATH", None)
            environment["PYTHONNOUSERSITE"] = "1"
            run_checked(
                [
                    sys.executable,
                    "-m",
                    "pip",
                    "wheel",
                    "--disable-pip-version-check",
                    "--no-deps",
                    "--no-build-isolation",
                    "--wheel-dir",
                    str(wheel_directory),
                    str(package_source),
                ],
                cwd=self.root,
                environment=environment,
            )
            wheels = list(wheel_directory.glob("mehlissa_research-1.0.0-*.whl"))
            self.assertEqual(len(wheels), 1)
            with zipfile.ZipFile(wheels[0]) as archive:
                names = set(archive.namelist())
            for asset in ("index.html", "styles.css", "app.js"):
                self.assertIn(f"mehlissa_workbench/static/{asset}", names)
            self.assertTrue(any(name.endswith("licenses/LICENSE.md") for name in names))

            virtual_environment = directory / "clean-venv"
            venv.EnvBuilder(with_pip=True, clear=True).create(virtual_environment)
            scripts = virtual_environment / ("Scripts" if os.name == "nt" else "bin")
            installed_python = scripts / ("python.exe" if os.name == "nt" else "python")
            launcher = scripts / (
                "mehlissa-workbench.exe" if os.name == "nt" else "mehlissa-workbench"
            )
            run_checked(
                [
                    str(installed_python),
                    "-m",
                    "pip",
                    "install",
                    "--disable-pip-version-check",
                    "--no-index",
                    "--no-deps",
                    str(wheels[0]),
                ],
                cwd=self.root,
                environment=environment,
            )
            self.assertTrue(launcher.is_file())
            self.assertEqual(
                run_checked(
                    [str(launcher), "--version"],
                    cwd=self.root,
                    environment=environment,
                ).strip(),
                "mehlissa-workbench 1.0.0",
            )
            resource_check = run_checked(
                [
                    str(installed_python),
                    "-c",
                    (
                        "from importlib.resources import files; "
                        "root=files('mehlissa_workbench.static'); "
                        "assert all(root.joinpath(x).is_file() for x in "
                        "('index.html','styles.css','app.js')); print('package_assets=ready')"
                    ),
                ],
                cwd=directory,
                environment=environment,
            )
            self.assertEqual(resource_check.strip(), "package_assets=ready")
            check_output = run_checked(
                [
                    str(launcher),
                    "--repository-root",
                    str(self.root),
                    "--executable",
                    str(self.executable),
                    "--workspace",
                    str(directory / "workspace"),
                    "--runs",
                    str(directory / "runs"),
                    "--check",
                ],
                cwd=directory,
                environment=environment,
            )
            expected_lines = {
                "workbench_status=ready",
                "workbench_version=1.0.0",
                "scenario_editing=true",
                "model_count=5",
                "example_count=10",
                "scenario_validation=valid",
                "run_plan_count=1",
            }
            self.assertTrue(expected_lines.issubset(set(check_output.splitlines())))


if __name__ == "__main__":
    unittest.main(argv=[sys.argv[0]], verbosity=2)
