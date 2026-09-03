# SPDX-FileCopyrightText: 2026 MEHLISSA contributors
# SPDX-License-Identifier: MPL-2.0

"""Loopback-only read-only host for the UX-6.1 workbench prototype."""

from __future__ import annotations

from dataclasses import asdict, dataclass
import hmac
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from importlib.resources import files
import json
from secrets import token_urlsafe
from typing import Callable, Sequence, cast
from urllib.parse import urlsplit

from mehlissa import MehlissaClient, MehlissaCommandError


CATALOG_API_VERSION = "1.0.0"
LOOPBACK_HOSTS = frozenset({"127.0.0.1", "localhost"})
STATIC_FILES = {
    "/": ("index.html", "text/html; charset=utf-8"),
    "/index.html": ("index.html", "text/html; charset=utf-8"),
    "/styles.css": ("styles.css", "text/css; charset=utf-8"),
    "/app.js": ("app.js", "text/javascript; charset=utf-8"),
}


class CatalogFormatError(RuntimeError):
    """Discovery output did not satisfy the UX-6.1 adapter contract."""


@dataclass(frozen=True)
class ModelSummary:
    """One model-family row returned by the accepted discovery command."""

    id: str
    layer: str
    maturity: str
    title: str


@dataclass(frozen=True)
class ExampleSummary:
    """One starter example plus model memberships resolved by discovery."""

    id: str
    path: str
    title: str
    model_ids: tuple[str, ...]


def _nonempty_lines(output: str) -> list[str]:
    return [line.strip() for line in output.splitlines() if line.strip()]


def _declared_count(lines: Sequence[str], key: str) -> int:
    prefix = f"{key}="
    values = [line.removeprefix(prefix) for line in lines if line.startswith(prefix)]
    if len(values) != 1:
        raise CatalogFormatError(f"Discovery output must contain exactly one {key}")
    try:
        value = int(values[0])
    except ValueError as error:
        raise CatalogFormatError(f"Discovery output contains an invalid {key}") from error
    if value < 0:
        raise CatalogFormatError(f"Discovery output contains a negative {key}")
    return value


def _parse_models(output: str) -> tuple[ModelSummary, ...]:
    lines = _nonempty_lines(output)
    expected = _declared_count(lines, "model_count")
    models: list[ModelSummary] = []
    for line in lines:
        if line.startswith("model_count="):
            continue
        fields = tuple(field.strip() for field in line.split(" | ", 3))
        if len(fields) != 4 or any(not field for field in fields):
            raise CatalogFormatError("Malformed model row in discovery output")
        models.append(ModelSummary(*fields))
    if len(models) != expected:
        raise CatalogFormatError("Model rows do not match model_count")
    if len({model.id for model in models}) != len(models):
        raise CatalogFormatError("Discovery output contains duplicate model ids")
    return tuple(models)


def _parse_examples(output: str) -> tuple[tuple[str, str, str], ...]:
    lines = _nonempty_lines(output)
    expected = _declared_count(lines, "example_count")
    examples: list[tuple[str, str, str]] = []
    for line in lines:
        if line.startswith("example_count="):
            continue
        fields = tuple(field.strip() for field in line.split(" | ", 2))
        if len(fields) != 3 or any(not field for field in fields):
            raise CatalogFormatError("Malformed example row in discovery output")
        examples.append(cast(tuple[str, str, str], fields))
    if len(examples) != expected:
        raise CatalogFormatError("Example rows do not match example_count")
    if len({example[0] for example in examples}) != len(examples):
        raise CatalogFormatError("Discovery output contains duplicate example ids")
    return tuple(examples)


def discover_catalog(client: MehlissaClient) -> dict[str, object]:
    """Return a structured projection obtained only through read-only commands."""

    models = _parse_models(client.list_models())
    all_examples = _parse_examples(client.list_examples())
    memberships: dict[str, list[str]] = {example[0]: [] for example in all_examples}

    for model in models:
        for example_id, _, _ in _parse_examples(client.list_examples(model.id)):
            if example_id not in memberships:
                raise CatalogFormatError(
                    "Filtered discovery returned an example absent from the full catalog"
                )
            memberships[example_id].append(model.id)

    examples = tuple(
        ExampleSummary(example_id, path, title, tuple(memberships[example_id]))
        for example_id, path, title in all_examples
    )
    return {
        "api_version": CATALOG_API_VERSION,
        "application": "MEHLISSA Next Research Workbench",
        "prototype": "UX-6.1",
        "read_only": True,
        "clinical_use": False,
        "models": [asdict(model) for model in models],
        "examples": [asdict(example) for example in examples],
    }


class WorkbenchServer(ThreadingHTTPServer):
    """HTTP server carrying the bounded workbench session state."""

    daemon_threads = True

    def __init__(
        self,
        server_address: tuple[str, int],
        client: MehlissaClient,
        session_token: str | None = None,
        catalog_loader: Callable[[MehlissaClient], dict[str, object]] = discover_catalog,
    ):
        host, _ = server_address
        if host not in LOOPBACK_HOSTS:
            raise ValueError("The UX-6.1 prototype may bind only to loopback")
        self.client = client
        self.session_token = session_token or token_urlsafe(32)
        self.catalog_loader = catalog_loader
        super().__init__(server_address, WorkbenchRequestHandler)

    @property
    def url(self) -> str:
        """Browser URL carrying an ephemeral capability for this local session."""

        host, port = self.server_address
        return f"http://{host}:{port}/?session={self.session_token}"


class WorkbenchRequestHandler(BaseHTTPRequestHandler):
    """Serve four embedded assets and one capability-protected read-only API."""

    server_version = "MEHLISSAWorkbench/0.1"
    sys_version = ""

    @property
    def workbench(self) -> WorkbenchServer:
        return cast(WorkbenchServer, self.server)

    def log_message(self, format: str, *args: object) -> None:
        # Do not place the session capability or future file paths in terminal logs.
        del format, args

    def _host_is_allowed(self) -> bool:
        host_header = self.headers.get("Host", "")
        host = host_header.rsplit(":", 1)[0].lower()
        return host in LOOPBACK_HOSTS

    def _security_headers(self) -> None:
        self.send_header(
            "Content-Security-Policy",
            "default-src 'self'; script-src 'self'; style-src 'self'; "
            "img-src 'self'; connect-src 'self'; base-uri 'none'; "
            "form-action 'none'; frame-ancestors 'none'",
        )
        self.send_header("Cache-Control", "no-store")
        self.send_header("Content-Language", "en")
        self.send_header("Referrer-Policy", "no-referrer")
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("X-Frame-Options", "DENY")
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Resource-Policy", "same-origin")
        self.send_header("Permissions-Policy", "camera=(), microphone=(), geolocation=()")

    def _send_bytes(self, status: HTTPStatus, content_type: str, body: bytes) -> None:
        self.send_response(status)
        self._security_headers()
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _send_json(self, status: HTTPStatus, document: dict[str, object]) -> None:
        body = json.dumps(document, ensure_ascii=False, separators=(",", ":")).encode("utf-8")
        self._send_bytes(status, "application/json; charset=utf-8", body)

    def do_GET(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        if not self._host_is_allowed():
            self._send_json(HTTPStatus.BAD_REQUEST, {"error": "invalid_host"})
            return

        path = urlsplit(self.path).path
        if path == "/api/catalog":
            supplied = self.headers.get("X-MEHLISSA-Session", "")
            if not hmac.compare_digest(supplied, self.workbench.session_token):
                self._send_json(HTTPStatus.FORBIDDEN, {"error": "session_required"})
                return
            try:
                payload = self.workbench.catalog_loader(self.workbench.client)
            except (CatalogFormatError, MehlissaCommandError, OSError):
                self._send_json(
                    HTTPStatus.BAD_GATEWAY,
                    {
                        "error": "catalog_unavailable",
                        "detail": "MEHLISSA discovery failed; inspect the local terminal",
                    },
                )
                return
            self._send_json(HTTPStatus.OK, payload)
            return

        static = STATIC_FILES.get(path)
        if static is None:
            self._send_json(HTTPStatus.NOT_FOUND, {"error": "not_found"})
            return
        asset, content_type = static
        body = files("mehlissa_workbench.static").joinpath(asset).read_bytes()
        self._send_bytes(HTTPStatus.OK, content_type, body)

    def _reject_state_changing_method(self) -> None:
        self.send_response(HTTPStatus.METHOD_NOT_ALLOWED)
        self._security_headers()
        self.send_header("Allow", "GET")
        self.send_header("Content-Length", "0")
        self.end_headers()

    def do_POST(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        self._reject_state_changing_method()

    def do_PUT(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        self._reject_state_changing_method()

    def do_PATCH(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        self._reject_state_changing_method()

    def do_DELETE(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        self._reject_state_changing_method()

    def do_OPTIONS(self) -> None:  # noqa: N802 - BaseHTTPRequestHandler contract
        self._reject_state_changing_method()


def create_server(
    client: MehlissaClient,
    host: str = "127.0.0.1",
    port: int = 8765,
    session_token: str | None = None,
    catalog_loader: Callable[[MehlissaClient], dict[str, object]] = discover_catalog,
) -> WorkbenchServer:
    """Create, but do not start, a loopback-only workbench server."""

    if not 0 <= port <= 65535:
        raise ValueError("port must be between 0 and 65535")
    return WorkbenchServer((host, port), client, session_token, catalog_loader)
