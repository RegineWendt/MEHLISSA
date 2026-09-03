# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Schema and semantic checks for the Paper 1 evidence/validity baseline."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

from jsonschema import Draft202012Validator, FormatChecker


REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MATRIX = REPOSITORY_ROOT / "data/evidence/evidence-validity-matrix-v1.json"
DEFAULT_SCHEMA = (
    REPOSITORY_ROOT / "data/schemas/evidence-validity-matrix/1.0.0.schema.json"
)
REQUIRED_FAMILIES = {
    "body",
    "organ-lung",
    "capillary-molecular",
    "cell-intracellular",
    "nano-iot",
    "m7-fp9-lung",
}


class MatrixError(ValueError):
    """Raised when schema-valid data fail cross-artifact semantics."""


def load_json(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as stream:
        document = json.load(stream)
    if not isinstance(document, dict):
        raise MatrixError(f"{path}: root must be an object")
    return document


def schema_errors(document: dict[str, Any], schema: dict[str, Any]) -> list[str]:
    validator = Draft202012Validator(schema, format_checker=FormatChecker())
    return [
        f"schema at {'/'.join(str(part) for part in error.absolute_path) or '<root>'}: "
        f"{error.message}"
        for error in sorted(validator.iter_errors(document), key=lambda item: list(item.path))
    ]


def _duplicates(values: list[str]) -> list[str]:
    return sorted({value for value in values if values.count(value) > 1})


def semantic_errors(document: dict[str, Any], repository_root: Path) -> list[str]:
    errors: list[str] = []
    sources = document.get("sources", [])
    models = document.get("model_families", [])
    source_ids = [item["id"] for item in sources]
    model_ids = [item["id"] for item in models]

    for duplicate in _duplicates(source_ids):
        errors.append(f"duplicate source id: {duplicate}")
    for duplicate in _duplicates(model_ids):
        errors.append(f"duplicate model id: {duplicate}")

    families = {item["family"] for item in models}
    if families != REQUIRED_FAMILIES:
        errors.append(
            "model family coverage differs: "
            f"missing={sorted(REQUIRED_FAMILIES - families)}, "
            f"extra={sorted(families - REQUIRED_FAMILIES)}"
        )

    known_sources = set(source_ids)
    referenced_sources: set[str] = set()
    paths: set[str] = set()

    for model in models:
        item_ids: list[str] = []
        for collection_name in ("outputs", "parameter_groups", "claims"):
            for item in model[collection_name]:
                item_ids.append(item["id"])
                for source_id in item.get("source_ids", []):
                    referenced_sources.add(source_id)
                    if source_id not in known_sources:
                        errors.append(
                            f"{model['id']}/{item['id']}: unknown source id {source_id}"
                        )
                for source_key in ("calibration_source_ids", "validation_source_ids"):
                    for source_id in item.get(source_key, []):
                        referenced_sources.add(source_id)
                        if source_id not in known_sources:
                            errors.append(
                                f"{model['id']}/{item['id']}: unknown {source_key} id {source_id}"
                            )

                if collection_name == "parameter_groups":
                    overlap = set(item["calibration_source_ids"]) & set(
                        item["validation_source_ids"]
                    )
                    assessment = item["source_overlap_assessment"].lower()
                    if overlap and not any(
                        marker in assessment
                        for marker in ("overlap", "same", "not independent", "reproduction")
                    ):
                        errors.append(
                            f"{model['id']}/{item['id']}: calibration/validation overlap "
                            f"{sorted(overlap)} is not explicitly assessed"
                        )

        for duplicate in _duplicates(item_ids):
            errors.append(f"{model['id']}: duplicate evidence item id {duplicate}")

        for input_item in model["inputs"]:
            paths.update(input_item["source_artifacts"])
        for collection_name in ("outputs", "parameter_groups"):
            for item in model[collection_name]:
                paths.update(item["artifacts"])
        for artifact_paths in model["artifacts"].values():
            paths.update(artifact_paths)

        for claim in model["claims"]:
            if claim["support"] == "supported" and claim["statement"].lower().find(
                "clinical"
            ) >= 0:
                errors.append(f"{model['id']}/{claim['id']}: clinical claim marked supported")

    for path_text in sorted(paths):
        path = repository_root / path_text
        if not path.is_file():
            errors.append(f"referenced artifact does not exist: {path_text}")

    audited_sources: set[str] = set()
    for audit in document.get("source_audit", []):
        source_id = audit["source_id"]
        audited_sources.add(source_id)
        if source_id not in known_sources:
            errors.append(f"source audit uses unknown source id: {source_id}")
        artifact = repository_root / audit["artifact"]
        if not artifact.is_file():
            errors.append(f"source audit artifact does not exist: {audit['artifact']}")
            continue
        if audit["artifact_source_id"] not in artifact.read_text(encoding="utf-8"):
            errors.append(
                f"source audit id {audit['artifact_source_id']} is absent from {audit['artifact']}"
            )

    external_used = {
        source["id"]
        for source in sources
        if source["kind"] in {"publication", "dataset"}
        and source["id"] in referenced_sources
    }
    if missing_audits := external_used - audited_sources:
        errors.append(f"externally used sources lack concrete-role audit: {sorted(missing_audits)}")

    if document["matrix"]["clinical_validity_claim"] is not False:
        errors.append("matrix must not make a clinical-validity claim")

    return errors


def validate(matrix_path: Path, schema_path: Path, repository_root: Path) -> None:
    document = load_json(matrix_path)
    schema = load_json(schema_path)
    errors = schema_errors(document, schema)
    if not errors:
        errors.extend(semantic_errors(document, repository_root))
    if errors:
        raise MatrixError("\n".join(errors))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--matrix", type=Path, default=DEFAULT_MATRIX)
    parser.add_argument("--schema", type=Path, default=DEFAULT_SCHEMA)
    args = parser.parse_args()
    try:
        validate(args.matrix.resolve(), args.schema.resolve(), REPOSITORY_ROOT)
    except (OSError, json.JSONDecodeError, MatrixError) as error:
        print(f"evidence/validity matrix: FAILED\n{error}")
        return 1
    document = load_json(args.matrix.resolve())
    print(
        "evidence/validity matrix: ok "
        f"({len(document['model_families'])} families, "
        f"{len(document['sources'])} sources, "
        f"{len(document['source_audit'])} source-role audits)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
