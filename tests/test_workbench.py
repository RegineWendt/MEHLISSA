# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

from __future__ import annotations

import argparse
from http import HTTPStatus
import json
from pathlib import Path
import sys
import threading
import unittest
from urllib.error import HTTPError
from urllib.request import Request, urlopen

from mehlissa import MehlissaClient
from mehlissa_workbench import CatalogFormatError, create_server, discover_catalog


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
        cls.server = create_server(client, port=0, session_token="test-session-token")
        cls.thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()
        host, port = cls.server.server_address
        cls.base_url = f"http://{host}:{port}"

    @classmethod
    def tearDownClass(cls) -> None:
        cls.server.shutdown()
        cls.server.server_close()
        cls.thread.join(timeout=5)

    def test_static_shell_is_accessible_and_security_hardened(self) -> None:
        with urlopen(f"{self.base_url}/?session=test-session-token", timeout=10) as response:
            body = response.read().decode("utf-8")
            self.assertEqual(response.status, HTTPStatus.OK)
            self.assertIn("MEHLISSA Next Research Workbench", body)
            self.assertIn("Read-only foundation", body)
            self.assertIn("frame-ancestors 'none'", response.headers["Content-Security-Policy"])
            self.assertEqual(response.headers["Referrer-Policy"], "no-referrer")
            self.assertEqual(response.headers["Cache-Control"], "no-store")
            self.assertEqual(response.headers["Content-Language"], "en")

        with urlopen(f"{self.base_url}/app.js", timeout=10) as response:
            script = response.read().decode("utf-8")
            self.assertNotIn("innerHTML", script)
            self.assertIn("X-MEHLISSA-Session", script)

    def test_catalog_api_requires_session_and_is_read_only(self) -> None:
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
            self.assertEqual(len(catalog["models"]), 5)

        for method in ("POST", "PUT", "PATCH", "DELETE", "OPTIONS"):
            with self.subTest(method=method):
                request = Request(f"{self.base_url}/api/catalog", data=b"", method=method)
                with self.assertRaises(HTTPError) as rejected:
                    urlopen(request, timeout=10)
                self.assertEqual(rejected.exception.code, HTTPStatus.METHOD_NOT_ALLOWED)
                self.assertEqual(rejected.exception.headers["Allow"], "GET")

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
