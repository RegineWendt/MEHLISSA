# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import argparse
from http import HTTPStatus
import json
from pathlib import Path
import sys
import tempfile
import threading
import time
import unittest
from urllib.error import HTTPError
from urllib.request import Request, urlopen

from mehlissa import MehlissaClient, MehlissaCommandError
from mehlissa_workbench import (
    CatalogFormatError,
    ScenarioWorkspace,
    ScenarioWorkspaceError,
    create_server,
    discover_catalog,
)


PARSER = argparse.ArgumentParser()
PARSER.add_argument("--executable", required=True)
PARSER.add_argument("--root", required=True)
ARGS = PARSER.parse_args()


class WorkbenchDiscoveryTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        cls.root = Path(ARGS.root).resolve()
        cls.client = MehlissaClient(ARGS.executable, cls.root)

    def test_structured_catalog_comes_from_accepted_discovery_commands(self) -> None:
        catalog = discover_catalog(self.client)
        self.assertEqual(catalog["api_version"], "1.0.0")
        self.assertTrue(catalog["read_only"])
        self.assertTrue(catalog["scenario_editing"])
        self.assertEqual(catalog["workbench_increment"], "UX-6.4")
        self.assertFalse(catalog["clinical_use"])
        self.assertEqual(len(catalog["models"]), 5)
        self.assertEqual(len(catalog["examples"]), 10)

        model_ids = {model["id"] for model in catalog["models"]}
        self.assertIn("organ.pulmonary-zero-dimensional", model_ids)
        fp9 = next(
            example for example in catalog["examples"] if example["id"] == "scenario.fp9-complete"
        )
        self.assertIn("organ.pulmonary-zero-dimensional", fp9["model_ids"])
        self.assertIn("nano-iot.communication", fp9["model_ids"])

    def test_malformed_discovery_output_fails_closed(self) -> None:
        class MalformedClient:
            def list_models(self) -> str:
                return "not a model row\nmodel_count=1\n"

        with self.assertRaises(CatalogFormatError):
            discover_catalog(MalformedClient())  # type: ignore[arg-type]


class WorkbenchServerTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        client = MehlissaClient(ARGS.executable, Path(ARGS.root).resolve())
        cls.client = client
        temporary_parent = Path(ARGS.root).resolve() / "tmp"
        temporary_parent.mkdir(exist_ok=True)
        cls.temporary_directory = tempfile.TemporaryDirectory(
            prefix="ux6-4-workspace-", dir=temporary_parent
        )
        cls.workspace = Path(cls.temporary_directory.name)
        cls.server = create_server(
            client,
            port=0,
            session_token="test-session-token",
            workspace_root=cls.workspace,
            runs_root=cls.workspace / "runs",
        )
        cls.thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()
        host, port = cls.server.server_address
        cls.base_url = f"http://{host}:{port}"

    @classmethod
    def tearDownClass(cls) -> None:
        cls.server.shutdown()
        cls.server.server_close()
        cls.thread.join(timeout=5)
        cls.temporary_directory.cleanup()

    def test_static_shell_is_accessible_and_security_hardened(self) -> None:
        with urlopen(f"{self.base_url}/?session=test-session-token", timeout=10) as response:
            body = response.read().decode("utf-8")
            self.assertEqual(response.status, HTTPStatus.OK)
            self.assertIn("MEHLISSA Next Research Workbench", body)
            self.assertIn("Confirm before execution", body)
            self.assertIn("Scenario validation", body)
            self.assertIn("frame-ancestors 'none'", response.headers["Content-Security-Policy"])
            self.assertEqual(response.headers["Referrer-Policy"], "no-referrer")
            self.assertEqual(response.headers["Cache-Control"], "no-store")
            self.assertEqual(response.headers["Content-Language"], "en")

        with urlopen(f"{self.base_url}/app.js", timeout=10) as response:
            script = response.read().decode("utf-8")
            self.assertNotIn("innerHTML", script)
            self.assertIn("X-MEHLISSA-Session", script)
            self.assertIn("beforeunload", script)
            self.assertIn("/api/scenario/validate", script)
            self.assertIn("/api/run/campaign", script)

    def test_catalog_api_requires_session(self) -> None:
        with self.assertRaises(HTTPError) as denied:
            urlopen(f"{self.base_url}/api/catalog", timeout=10)
        self.assertEqual(denied.exception.code, HTTPStatus.FORBIDDEN)

        request = Request(
            f"{self.base_url}/api/catalog",
            headers={"X-MEHLISSA-Session": "test-session-token"},
        )
        with urlopen(request, timeout=20) as response:
            catalog = json.load(response)
            self.assertEqual(response.status, HTTPStatus.OK)
            self.assertTrue(catalog["read_only"])
            self.assertTrue(catalog["scenario_editing"])
            self.assertEqual(len(catalog["models"]), 5)

        for method in ("PUT", "PATCH", "DELETE", "OPTIONS"):
            with self.subTest(method=method):
                request = Request(f"{self.base_url}/api/catalog", data=b"", method=method)
                with self.assertRaises(HTTPError) as rejected:
                    urlopen(request, timeout=10)
                self.assertEqual(rejected.exception.code, HTTPStatus.METHOD_NOT_ALLOWED)
                self.assertEqual(rejected.exception.headers["Allow"], "GET, POST")

    def request_json(self, path: str, method: str = "GET", body: object | None = None):
        data = json.dumps(body).encode("utf-8") if body is not None else None
        headers = {"X-MEHLISSA-Session": "test-session-token"}
        if data is not None:
            headers["Content-Type"] = "application/json"
        request = Request(f"{self.base_url}{path}", headers=headers, data=data, method=method)
        with urlopen(request, timeout=20) as response:
            return response.status, json.load(response)

    def wait_job(self, job_id: str, timeout: float = 40) -> dict[str, object]:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            _, job = self.request_json(f"/api/run?id={job_id}")
            if job["status"] not in {"queued", "running", "collecting"}:
                return job
            time.sleep(0.1)
        self.fail(f"workbench run {job_id} did not finish")

    def test_guided_scenario_round_trip_retains_complete_document(self) -> None:
        status, overview = self.request_json("/api/scenarios")
        self.assertEqual(status, HTTPStatus.OK)
        self.assertEqual(overview["save_policy"], "save_as_only")
        self.assertEqual(len(overview["sources"]), 1)

        source_id = overview["sources"][0]["id"]
        _, original = self.request_json(f"/api/scenario?id={source_id}")
        paths = {field["path"] for field in original["fields"]}
        self.assertIn("run.collector_count", paths)
        self.assertIn("The dissertation baseline", next(
            field["evidence"]
            for field in original["fields"]
            if field["path"] == "run.collector_count"
        ))
        self.assertEqual(len(original["document"]["artifacts"]), 13)

        filename = "ux6-2-round-trip.json"
        status, saved = self.request_json(
            "/api/scenario/save",
            "POST",
            {
                "source_id": source_id,
                "filename": filename,
                "changes": {
                    "scenario.id": "fp9-lung-ux6-2-round-trip",
                    "scenario.title": "UX-6.2 round-trip scenario",
                    "run.id": "fp9-lung-ux6-2-collectors-10000",
                    "run.collector_count": 10000,
                },
            },
        )
        self.assertEqual(status, HTTPStatus.CREATED)
        self.assertEqual(saved["document"]["run"]["collector_count"], 10000)
        self.assertEqual(saved["document"]["artifacts"], original["document"]["artifacts"])
        self.assertEqual(saved["document"]["sources"], original["document"]["sources"])

        _, reopened = self.request_json(f"/api/scenario?id=saved:{filename}")
        self.assertEqual(reopened["document"], saved["document"])

        with self.assertRaises(HTTPError) as conflict:
            self.request_json(
                "/api/scenario/save",
                "POST",
                {"source_id": source_id, "filename": filename, "changes": {}},
            )
        self.assertEqual(conflict.exception.code, HTTPStatus.CONFLICT)

    def test_validation_matches_cli_and_reports_field_repairs(self) -> None:
        source_id = "template:scenario.fp9-complete"
        status, positive = self.request_json(
            "/api/scenario/validate", "POST", {"source_id": source_id, "changes": {}}
        )
        self.assertEqual(status, HTTPStatus.OK)
        self.assertTrue(positive["valid"])
        self.assertTrue(positive["run_allowed"])
        self.assertEqual(positive["validator"], "mehlissa scenario validate")
        self.assertEqual(positive["error_count"], 0)
        self.assertIn("Status: VALID", positive["summary_text"])

        changes = {"run.collector_count": 0}
        _, negative = self.request_json(
            "/api/scenario/validate", "POST", {"source_id": source_id, "changes": changes}
        )
        self.assertFalse(negative["valid"])
        self.assertFalse(negative["run_allowed"])
        field_issues = [
            issue for issue in negative["issues"]
            if issue["path"] == "run.collector_count"
        ]
        self.assertTrue(any(issue["code"] == "WBV-1003" for issue in field_issues))
        self.assertTrue(any(issue["code"] == "MEHLISSA-E2005" for issue in field_issues))
        self.assertTrue(all(issue["guidance"] for issue in field_issues))
        self.assertNotIn(".mehlissa-validation-", negative["summary_text"])

        candidate = json.loads(
            (Path(ARGS.root) / "examples/scenarios/fp9-lung-level-a-v1.json").read_text(
                encoding="utf-8"
            )
        )
        candidate["run"]["collector_count"] = 0
        candidate_path = self.workspace / "cli-parity-invalid.json"
        candidate_path.write_text(json.dumps(candidate), encoding="utf-8")
        with self.assertRaises(MehlissaCommandError):
            self.client.validate_scenario(candidate_path)

    def test_validation_distinguishes_warnings_from_errors(self) -> None:
        _, report = self.request_json(
            "/api/scenario/validate",
            "POST",
            {
                "source_id": "template:scenario.fp9-complete",
                "changes": {"run.master_seed": 77},
            },
        )
        self.assertTrue(report["valid"])
        self.assertTrue(report["run_allowed"])
        self.assertEqual(report["error_count"], 0)
        self.assertEqual(report["warning_count"], 1)
        self.assertEqual(
            {issue["code"] for issue in report["issues"]}, {"WBV-2002"}
        )

    def test_semantic_and_cross_file_failures_have_locations(self) -> None:
        _, target = self.request_json(
            "/api/scenario/validate",
            "POST",
            {
                "source_id": "template:scenario.fp9-complete",
                "changes": {"target.tissue": "kidney"},
            },
        )
        self.assertFalse(target["valid"])
        self.assertEqual(target["issues"][-1]["path"], "target.tissue")

        template = Path(ARGS.root) / "examples/scenarios/fp9-lung-level-a-v1.json"
        document = json.loads(template.read_text(encoding="utf-8"))
        document["artifacts"][0]["definition_path"] = "data/body-models/missing.json"
        broken = self.workspace / "broken-artifact.json"
        broken.write_text(json.dumps(document), encoding="utf-8")
        _, cross_file = self.request_json(
            "/api/scenario/validate",
            "POST",
            {"source_id": "saved:broken-artifact.json", "changes": {}},
        )
        self.assertFalse(cross_file["valid"])
        self.assertFalse(cross_file["run_allowed"])
        self.assertEqual(cross_file["issues"][-1]["path"], "artifacts")
        self.assertEqual(cross_file["issues"][-1]["code"], "MEHLISSA-E2001")

    def test_invalid_candidate_cannot_be_saved_or_started(self) -> None:
        request = Request(
            f"{self.base_url}/api/scenario/save",
            headers={
                "X-MEHLISSA-Session": "test-session-token",
                "Content-Type": "application/json",
            },
            data=json.dumps(
                {
                    "source_id": "template:scenario.fp9-complete",
                    "filename": "invalid-must-not-save.json",
                    "changes": {"run.collector_count": 0},
                }
            ).encode("utf-8"),
            method="POST",
        )
        with self.assertRaises(HTTPError) as rejected:
            urlopen(request, timeout=20)
        self.assertEqual(rejected.exception.code, HTTPStatus.UNPROCESSABLE_ENTITY)
        payload = json.load(rejected.exception)
        self.assertEqual(payload["error"], "scenario_invalid")
        self.assertFalse(payload["validation"]["run_allowed"])
        self.assertFalse((self.workspace / "invalid-must-not-save.json").exists())

        with self.assertRaises(HTTPError) as rejected_start:
            self.request_json(
                "/api/run/scenario", "POST",
                {
                    "source_id": "template:scenario.fp9-complete",
                    "changes": {"run.collector_count": 0},
                    "output_label": "invalid",
                    "confirmed": True,
                },
            )
        self.assertEqual(rejected_start.exception.code, HTTPStatus.UNPROCESSABLE_ENTITY)
        _, runs = self.request_json("/api/runs")
        self.assertFalse(any(job["directory"].endswith("invalid") for job in runs["jobs"]))

    def test_run_plan_requires_confirmation_and_safe_output_label(self) -> None:
        _, plans = self.request_json("/api/run-plans")
        self.assertEqual(plans["api_version"], "1.0.0")
        self.assertEqual(plans["campaigns"][0]["run_count"], 6)
        self.assertEqual(len(plans["campaigns"][0]["manifest_sha256"]), 64)
        self.assertEqual(plans["campaigns"][0]["replicates"]["count"], 2)
        self.assertTrue(plans["campaigns"][0]["sweeps"])
        self.assertTrue(plans["campaigns"][0]["paired_comparisons"])
        for body in (
            {"source_id": "template:scenario.fp9-complete", "changes": {}, "output_label": "unconfirmed", "confirmed": False},
            {"source_id": "template:scenario.fp9-complete", "changes": {}, "output_label": "../escape", "confirmed": True},
        ):
            with self.subTest(body=body), self.assertRaises(HTTPError) as rejected:
                self.request_json("/api/run/scenario", "POST", body)
            self.assertEqual(rejected.exception.code, HTTPStatus.BAD_REQUEST)

    def test_reference_scenario_runs_with_retained_trace(self) -> None:
        status, started = self.request_json(
            "/api/run/scenario", "POST",
            {
                "source_id": "template:scenario.fp9-complete",
                "changes": {}, "output_label": "reference-scenario", "confirmed": True,
            },
        )
        self.assertEqual(status, HTTPStatus.ACCEPTED)
        job = self.wait_job(started["id"])
        self.assertEqual(job["status"], "completed")
        self.assertEqual(job["progress_percent"], 100)
        self.assertEqual(job["plan"]["run_count"], 1)
        names = {artifact["name"] for artifact in job["artifacts"]}
        self.assertTrue({"run-record", "input", "command-log", "result", "provenance", "simulation-log", "summary"}.issubset(names))
        self.assertLessEqual(len(job["logs"]), 200)
        request = Request(
            f"{self.base_url}/api/run/artifact?id={job['id']}&name=provenance",
            headers={"X-MEHLISSA-Session": "test-session-token"},
        )
        with urlopen(request, timeout=10) as response:
            provenance = json.load(response)
        self.assertIn("scenario", provenance)

    def test_six_run_campaign_completes_and_preserves_design(self) -> None:
        status, started = self.request_json(
            "/api/run/campaign", "POST",
            {"campaign_id": "campaign.fp9-collector-count", "output_label": "six-run-campaign", "confirmed": True},
        )
        self.assertEqual(status, HTTPStatus.ACCEPTED)
        job = self.wait_job(started["id"], timeout=60)
        self.assertEqual(job["status"], "completed")
        self.assertEqual(job["plan"]["run_count"], 6)
        self.assertEqual(job["plan"]["paired_comparisons"][0]["first_seed"], 20260930)
        names = {artifact["name"] for artifact in job["artifacts"]}
        self.assertTrue({"input", "result", "csv", "command-log"}.issubset(names))
        self.assertEqual(len([name for name in names if name.startswith("derived-manifest-")]), 6)

    def test_campaign_cancellation_preserves_auditable_state(self) -> None:
        _, started = self.request_json(
            "/api/run/campaign", "POST",
            {"campaign_id": "campaign.fp9-collector-count", "output_label": "cancelled-campaign", "confirmed": True},
        )
        status, _ = self.request_json("/api/run/cancel", "POST", {"id": started["id"]})
        self.assertEqual(status, HTTPStatus.ACCEPTED)
        job = self.wait_job(started["id"], timeout=20)
        self.assertEqual(job["status"], "cancelled")
        self.assertTrue(any(artifact["name"] == "run-record" for artifact in job["artifacts"]))
        self.assertTrue(job["cancel_requested"])
        self.assertIn("preserved", job["stage"])

    def test_unknown_run_and_artifact_are_not_exposed(self) -> None:
        for path in ("/api/run?id=missing", "/api/run/artifact?id=missing&name=../input"):
            with self.subTest(path=path), self.assertRaises(HTTPError) as missing:
                self.request_json(path)
            self.assertEqual(missing.exception.code, HTTPStatus.NOT_FOUND)

    def test_unsupported_fields_are_visible_and_never_silently_discarded(self) -> None:
        template = Path(ARGS.root).resolve() / "examples/scenarios/fp9-lung-level-a-v1.json"
        document = json.loads(template.read_text(encoding="utf-8"))
        document["future_extension"] = {"retained": True}
        future = self.workspace / "future-compatible.json"
        future.write_text(json.dumps(document), encoding="utf-8")

        _, loaded = self.request_json("/api/scenario?id=saved:future-compatible.json")
        self.assertIn("future_extension", loaded["unknown_paths"])
        self.assertTrue(loaded["document"]["future_extension"]["retained"])

        with self.assertRaises(HTTPError) as rejected:
            self.request_json(
                "/api/scenario/save",
                "POST",
                {
                    "source_id": "saved:future-compatible.json",
                    "filename": "must-not-save.json",
                    "changes": {"future_extension.retained": False},
                },
            )
        self.assertEqual(rejected.exception.code, HTTPStatus.BAD_REQUEST)
        self.assertFalse((self.workspace / "must-not-save.json").exists())

    def test_workspace_rejects_escape_and_arbitrary_source_ids(self) -> None:
        client = MehlissaClient(ARGS.executable, Path(ARGS.root).resolve())
        with self.assertRaises(ScenarioWorkspaceError):
            ScenarioWorkspace(client, Path(ARGS.root).resolve())
        with self.assertRaises(ScenarioWorkspaceError):
            ScenarioWorkspace(client, Path(ARGS.root).resolve().parent / "outside")
        with self.assertRaises(HTTPError) as rejected:
            self.request_json("/api/scenario?id=saved:../pyproject.toml")
        self.assertEqual(rejected.exception.code, HTTPStatus.BAD_REQUEST)

    def test_untrusted_host_header_is_rejected(self) -> None:
        request = Request(
            f"{self.base_url}/api/catalog",
            headers={
                "Host": "untrusted.example",
                "X-MEHLISSA-Session": "test-session-token",
            },
        )
        with self.assertRaises(HTTPError) as rejected:
            urlopen(request, timeout=10)
        self.assertEqual(rejected.exception.code, HTTPStatus.BAD_REQUEST)

    def test_unknown_and_traversal_paths_are_not_served(self) -> None:
        for path in ("/missing", "/%2e%2e/pyproject.toml"):
            with self.subTest(path=path), self.assertRaises(HTTPError) as missing:
                urlopen(f"{self.base_url}{path}", timeout=10)
            self.assertEqual(missing.exception.code, HTTPStatus.NOT_FOUND)

    def test_non_loopback_binding_is_rejected(self) -> None:
        client = MehlissaClient(ARGS.executable, Path(ARGS.root).resolve())
        with self.assertRaises(ValueError):
            create_server(client, host="0.0.0.0", port=0)


if __name__ == "__main__":
    unittest.main(argv=[sys.argv[0]], verbosity=2)
