# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Local graphical research workbench for MEHLISSA Next."""

from .server import (
    CatalogFormatError,
    ExampleSummary,
    ModelSummary,
    WorkbenchServer,
    create_server,
    discover_catalog,
)

__all__ = [
    "CatalogFormatError",
    "ExampleSummary",
    "ModelSummary",
    "WorkbenchServer",
    "create_server",
    "discover_catalog",
]

__version__ = "0.1.0"
