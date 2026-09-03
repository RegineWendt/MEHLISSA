# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Stable Python access to MEHLISSA commands and versioned results."""

from .client import (
    CampaignExecution,
    MehlissaClient,
    MehlissaCancelledError,
    MehlissaCommandError,
    ReportBundle,
    ScenarioExecution,
)
from .results import CampaignResult, ScenarioResult, load_campaign_result, load_result

__all__ = [
    "CampaignExecution",
    "CampaignResult",
    "MehlissaClient",
    "MehlissaCancelledError",
    "MehlissaCommandError",
    "ReportBundle",
    "ScenarioExecution",
    "ScenarioResult",
    "load_campaign_result",
    "load_result",
]

__version__ = "0.1.0"
