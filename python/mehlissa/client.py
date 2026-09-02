# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Subprocess client for the authoritative MEHLISSA C++ application."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import subprocess
from typing import Iterable


class MehlissaCommandError(RuntimeError):
    """A MEHLISSA command returned a non-zero exit status."""

    def __init__(self, command: Iterable[str], returncode: int, stdout: str, stderr: str):
        self.command = tuple(command)
        self.returncode = returncode
        self.stdout = stdout
        self.stderr = stderr
        detail = stderr.strip() or stdout.strip() or "no diagnostic output"
        super().__init__(f"MEHLISSA command failed with status {returncode}: {detail}")


@dataclass(frozen=True)
class ScenarioExecution:
    directory: Path
    result: Path
    provenance: Path
    log: Path
    summary: Path
    stdout: str


@dataclass(frozen=True)
class ReportBundle:
    directory: Path
    html: Path
    result: Path
    stdout: str


@dataclass(frozen=True)
class CampaignExecution:
    directory: Path
    result: Path
    csv: Path
    derived_runs: int
    stdout: str


def _properties(output: str) -> dict[str, str]:
    properties: dict[str, str] = {}
    for line in output.splitlines():
        key, separator, value = line.partition("=")
        if separator and key and " " not in key:
            properties[key] = value
    return properties


class MehlissaClient:
    """Call MEHLISSA without reimplementing its validation or simulation logic."""

    def __init__(self, executable: str | Path, repository_root: str | Path | None = None):
        self.executable = Path(executable).expanduser().resolve()
        if not self.executable.is_file():
            raise FileNotFoundError(f"MEHLISSA executable not found: {self.executable}")
        self.repository_root = (
            Path(repository_root).expanduser().resolve() if repository_root is not None else None
        )

    def _run(self, *arguments: str | Path) -> subprocess.CompletedProcess[str]:
        command = [str(self.executable), *(str(argument) for argument in arguments)]
        completed = subprocess.run(command, capture_output=True, text=True, check=False)
        if completed.returncode != 0:
            raise MehlissaCommandError(
                command, completed.returncode, completed.stdout, completed.stderr
            )
        return completed

    def _root_arguments(self) -> list[str]:
        return (
            ["--repository-root", str(self.repository_root)]
            if self.repository_root is not None
            else []
        )

    def list_models(self, layer: str | None = None) -> str:
        arguments: list[str] = ["model", "list"]
        if layer is not None:
            arguments.extend(["--layer", layer])
        arguments.extend(self._root_arguments())
        return self._run(*arguments).stdout

    def describe_model(self, model_id: str) -> str:
        return self._run(
            "model", "describe", "--id", model_id, *self._root_arguments()
        ).stdout

    def list_examples(self, model_id: str | None = None) -> str:
        arguments: list[str] = ["example", "list"]
        if model_id is not None:
            arguments.extend(["--model", model_id])
        arguments.extend(self._root_arguments())
        return self._run(*arguments).stdout

    def list_scenarios(self) -> str:
        return self._run("scenario", "list", *self._root_arguments()).stdout

    def validate_scenario(self, file: str | Path) -> str:
        return self._run(
            "scenario", "validate", "--file", file, *self._root_arguments()
        ).stdout

    def run_scenario(self, file: str | Path, output: str | Path) -> ScenarioExecution:
        completed = self._run(
            "scenario",
            "run",
            "--file",
            file,
            "--output",
            output,
            *self._root_arguments(),
        )
        values = _properties(completed.stdout)
        required = (
            "run_directory",
            "result_file",
            "provenance_file",
            "run_log_file",
            "summary_file",
        )
        missing = [key for key in required if key not in values]
        if missing:
            raise RuntimeError(f"MEHLISSA scenario output omitted: {', '.join(missing)}")
        return ScenarioExecution(
            directory=Path(values["run_directory"]),
            result=Path(values["result_file"]),
            provenance=Path(values["provenance_file"]),
            log=Path(values["run_log_file"]),
            summary=Path(values["summary_file"]),
            stdout=completed.stdout,
        )

    def summarize_result(self, file: str | Path) -> str:
        return self._run(
            "result", "summarize", "--file", file, *self._root_arguments()
        ).stdout

    def report_result(self, file: str | Path, output: str | Path) -> ReportBundle:
        completed = self._run(
            "result",
            "report",
            "--file",
            file,
            "--output",
            output,
            *self._root_arguments(),
        )
        values = _properties(completed.stdout)
        directory = Path(values.get("report_directory", output))
        return ReportBundle(
            directory=directory,
            html=Path(values.get("html_report", directory / "report.html")),
            result=directory / "result.json",
            stdout=completed.stdout,
        )

    def validate_campaign(self, file: str | Path) -> str:
        return self._run(
            "campaign", "validate", "--file", file, *self._root_arguments()
        ).stdout

    def run_campaign(self, file: str | Path, output: str | Path) -> CampaignExecution:
        completed = self._run(
            "campaign",
            "run",
            "--file",
            file,
            "--output",
            output,
            *self._root_arguments(),
        )
        values = _properties(completed.stdout)
        required = ("campaign_directory", "campaign_result", "campaign_csv", "derived_runs")
        missing = [key for key in required if key not in values]
        if missing:
            raise RuntimeError(f"MEHLISSA campaign output omitted: {', '.join(missing)}")
        return CampaignExecution(
            directory=Path(values["campaign_directory"]),
            result=Path(values["campaign_result"]),
            csv=Path(values["campaign_csv"]),
            derived_runs=int(values["derived_runs"]),
            stdout=completed.stdout,
        )
