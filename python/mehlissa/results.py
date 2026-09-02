# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Version-aware readers and small analysis helpers for MEHLISSA result JSON."""

from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
from typing import Any, Iterable


def _read_object(path: str | Path) -> tuple[Path, dict[str, Any]]:
    resolved = Path(path).expanduser().resolve()
    with resolved.open("r", encoding="utf-8") as stream:
        value = json.load(stream)
    if not isinstance(value, dict):
        raise ValueError(f"Expected a JSON object in {resolved}")
    return resolved, value


def _require(document: dict[str, Any], keys: Iterable[str], role: str) -> None:
    missing = [key for key in keys if key not in document]
    if missing:
        raise ValueError(f"{role} is missing required fields: {', '.join(missing)}")


@dataclass(frozen=True)
class ScenarioResult:
    path: Path
    document: dict[str, Any]

    @property
    def summary(self) -> dict[str, Any]:
        analysis = self.document["level_e_analysis"]["summary"]
        return {
            "scenario_id": self.document["scenario"]["id"],
            "run_id": self.document["run"]["id"],
            "seed": self.document["run"]["master_seed"],
            "collector_count": self.document["run"]["collector_count"],
            "detected": self.document["level_b_detection"]["detected"],
            "assembled": self.document["level_c_assembly"]["complete"],
            "sensitivity": _estimate(analysis["sensitivity"]),
            "specificity": _estimate(analysis["specificity"]),
            "clinical_validation_claim": self.document["validity"][
                "clinical_validation_claim"
            ],
        }

    @property
    def runtime_stages(self) -> list[dict[str, Any]]:
        return list(self.document["runtime"]["stages"])

    @property
    def analysis_cases(self) -> list[dict[str, Any]]:
        return list(self.document["level_e_analysis"]["cases"])

    def plot_runtime(self, ax: Any = None) -> Any:
        """Plot stage times. Matplotlib is imported only when this method is called."""
        pyplot = _pyplot()
        if ax is None:
            _, ax = pyplot.subplots(figsize=(9, 4.5))
        names = [stage["stage"] for stage in self.runtime_stages]
        times = [stage["time_ns"] / 1_000_000 for stage in self.runtime_stages]
        ax.barh(names, times, color="#1f8f98")
        ax.invert_yaxis()
        ax.set_xlabel("Cumulative scenario time (ms)")
        ax.set_title(f"MEHLISSA runtime stages: {self.summary['run_id']}")
        return ax


@dataclass(frozen=True)
class CampaignResult:
    path: Path
    document: dict[str, Any]

    @property
    def runs(self) -> list[dict[str, Any]]:
        return list(self.document["runs"])

    def metric_series(self, metric: str) -> list[tuple[int, float | bool | None]]:
        if metric not in {"detected", "assembled", "sensitivity", "specificity"}:
            raise ValueError(f"Unsupported campaign metric: {metric}")
        return [(run["value"], run[metric]) for run in self.runs]

    def groups(self) -> dict[str, list[dict[str, Any]]]:
        grouped: dict[str, list[dict[str, Any]]] = {}
        for run in self.runs:
            grouped.setdefault(run["group"], []).append(run)
        return grouped

    def paired_differences(self, metric: str) -> list[dict[str, Any]]:
        if metric not in {"detected", "assembled", "sensitivity", "specificity"}:
            raise ValueError(f"Unsupported campaign metric: {metric}")
        indexed: dict[tuple[str, int, int], dict[str, dict[str, Any]]] = {}
        for run in self.runs:
            if run["design"] != "paired_comparison":
                continue
            key = (run["group"], run["replicate_index"], run["seed"])
            indexed.setdefault(key, {})[run["role"]] = run
        differences: list[dict[str, Any]] = []
        for (group, replicate, seed), pair in sorted(indexed.items()):
            if set(pair) != {"baseline", "comparison"}:
                raise ValueError(f"Incomplete paired comparison: {group} replicate {replicate}")
            baseline = pair["baseline"][metric]
            comparison = pair["comparison"][metric]
            difference = (
                None
                if baseline is None or comparison is None
                else float(comparison) - float(baseline)
            )
            differences.append(
                {
                    "group": group,
                    "replicate_index": replicate,
                    "seed": seed,
                    "metric": metric,
                    "baseline": baseline,
                    "comparison": comparison,
                    "difference": difference,
                }
            )
        return differences

    def plot_metric(self, metric: str = "sensitivity", ax: Any = None) -> Any:
        """Plot a campaign response against collector count."""
        pyplot = _pyplot()
        if ax is None:
            _, ax = pyplot.subplots(figsize=(7, 4.5))
        series = [(value, response) for value, response in self.metric_series(metric)
                  if response is not None]
        ax.scatter([value for value, _ in series], [float(response) for _, response in series],
                   color="#1f8f98")
        ax.set_xscale("log")
        ax.set_xlabel("Collector count")
        ax.set_ylabel(metric.replace("_", " ").title())
        ax.set_title(self.document["campaign"]["title"])
        return ax


def _estimate(interval: dict[str, Any] | None) -> float | None:
    return None if interval is None else float(interval["estimate"])


def _pyplot() -> Any:
    try:
        from matplotlib import pyplot
    except ImportError as error:
        raise RuntimeError(
            "Plotting requires the optional dependency: pip install 'mehlissa-research[plot]'"
        ) from error
    return pyplot


def load_result(path: str | Path) -> ScenarioResult:
    resolved, document = _read_object(path)
    _require(
        document,
        (
            "schema_version",
            "scenario",
            "run",
            "runtime",
            "validity",
            "level_b_detection",
            "level_c_assembly",
            "level_e_analysis",
        ),
        "Scenario result",
    )
    if document["schema_version"] != "2.0.0":
        raise ValueError(
            f"Unsupported fingerprinting result schema version: {document['schema_version']}"
        )
    return ScenarioResult(resolved, document)


def load_campaign_result(path: str | Path) -> CampaignResult:
    resolved, document = _read_object(path)
    _require(
        document,
        ("schema_version", "campaign", "run_count", "runs", "sensitivity_hooks", "limitations"),
        "Campaign result",
    )
    if document["schema_version"] != "1.0.0":
        raise ValueError(f"Unsupported campaign result schema version: {document['schema_version']}")
    if document["run_count"] != len(document["runs"]):
        raise ValueError("Campaign run_count does not match the runs array")
    return CampaignResult(resolved, document)
