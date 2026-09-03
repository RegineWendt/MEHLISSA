# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Local graphical research workbench for MEHLISSA Next."""

from .server import (
    CatalogFormatError,
    ExampleSummary,
    ModelSummary,
    ScenarioConflictError,
    ScenarioWorkspace,
    ScenarioWorkspaceError,
    WorkbenchServer,
    create_server,
    discover_catalog,
    scenario_fields,
)

__all__ = [
    "CatalogFormatError",
    "ExampleSummary",
    "ModelSummary",
    "ScenarioConflictError",
    "ScenarioWorkspace",
    "ScenarioWorkspaceError",
    "WorkbenchServer",
    "create_server",
    "discover_catalog",
    "scenario_fields",
]

__version__ = "0.2.0"
