# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Outcome-blind MRSQ cohort statistics; contains no data-access code."""

from __future__ import annotations
from dataclasses import dataclass
import math
import random
from typing import Sequence


@dataclass(frozen=True)
class ParticipantMetric:
    normalized_rmse: float
    auc_ratio: float
    peak_time_error_fraction: float


@dataclass(frozen=True)
class CohortResult:
    complete_participants: int
    median_nrmse_upper_90_ci: float
    geometric_mean_auc_ratio_lower_90_ci: float
    geometric_mean_auc_ratio_upper_90_ci: float
    median_peak_time_error_fraction_upper_90_ci: float
    status: str


def participant_metric(predicted: Sequence[float], observed: Sequence[float], durations: Sequence[float],
                       midpoints: Sequence[float] | None = None) -> ParticipantMetric:
    if not predicted or len(predicted) != len(observed) or len(observed) != len(durations):
        raise ValueError("aligned non-empty vectors are required")
    if any(not math.isfinite(x) or x < 0.0 for x in (*predicted, *observed)) or any(d <= 0.0 for d in durations):
        raise ValueError("finite nonnegative observations and positive durations are required")
    total = sum(durations)
    if midpoints is None:
        elapsed = 0.0; generated: list[float] = []
        for duration in durations:
            generated.append(elapsed + duration / 2.0); elapsed += duration
        midpoints = generated
    if len(midpoints) != len(observed) or any(not math.isfinite(x) for x in midpoints) or any(b <= a for a, b in zip(midpoints, midpoints[1:])):
        raise ValueError("strictly increasing aligned frame midpoints are required")
    observed_scale = max(abs(value) for value in observed)
    observed_auc = sum((observed[i] + observed[i + 1]) * (midpoints[i + 1] - midpoints[i]) / 2.0 for i in range(len(observed) - 1))
    predicted_auc = sum((predicted[i] + predicted[i + 1]) * (midpoints[i + 1] - midpoints[i]) / 2.0 for i in range(len(predicted) - 1))
    if observed_scale <= 0.0 or observed_auc <= 0.0:
        raise ValueError("positive observed normalization is required")
    rmse = math.sqrt(sum(d * (x - y) ** 2 for x, y, d in zip(predicted, observed, durations)) / total)
    predicted_peak = max(range(len(predicted)), key=lambda index: predicted[index])
    observed_peak = max(range(len(observed)), key=lambda index: observed[index])
    peak_error = abs(midpoints[predicted_peak] - midpoints[observed_peak])
    peak_limit = max(10.0, durations[observed_peak])
    return ParticipantMetric(rmse / observed_scale, predicted_auc / observed_auc, peak_error / peak_limit)


def _percentile(values: Sequence[float], probability: float) -> float:
    ordered = sorted(values)
    position = (len(ordered) - 1) * probability
    lower = int(math.floor(position)); upper = int(math.ceil(position))
    return ordered[lower] if lower == upper else ordered[lower] * (upper - position) + ordered[upper] * (position - lower)


def cohort_result(metrics: Sequence[ParticipantMetric], *, nrmse_limit: float, ratio_lower: float,
                  ratio_upper: float, require_peak: bool = False, seed: int = 18042026,
                  replicates: int = 10000) -> CohortResult:
    if not metrics or replicates < 100:
        raise ValueError("metrics and at least 100 bootstrap replicates are required")
    if any(m.normalized_rmse < 0.0 or m.auc_ratio <= 0.0 or m.peak_time_error_fraction < 0.0 for m in metrics):
        raise ValueError("invalid participant metric")
    rng = random.Random(seed); count = len(metrics)
    medians: list[float] = []; geometric: list[float] = []; peak_medians: list[float] = []
    for _ in range(replicates):
        sample = [metrics[rng.randrange(count)] for _ in range(count)]
        medians.append(_percentile([m.normalized_rmse for m in sample], 0.5))
        geometric.append(math.exp(sum(math.log(m.auc_ratio) for m in sample) / count))
        peak_medians.append(_percentile([m.peak_time_error_fraction for m in sample], 0.5))
    upper_nrmse = _percentile(medians, 0.95)
    lower_ratio = _percentile(geometric, 0.05); upper_ratio = _percentile(geometric, 0.95)
    upper_peak = _percentile(peak_medians, 0.95)
    if count < 20: status = "BLOCKED"
    elif count < 60: status = "PARTIAL"
    elif (upper_nrmse <= nrmse_limit and lower_ratio >= ratio_lower and upper_ratio <= ratio_upper
          and (not require_peak or upper_peak <= 1.0)): status = "PASS"
    else: status = "FAIL"
    return CohortResult(count, upper_nrmse, lower_ratio, upper_ratio, upper_peak, status)
