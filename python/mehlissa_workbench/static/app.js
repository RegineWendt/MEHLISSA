// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

"use strict";

const state = { models: [], examples: [], session: "", workspace: null, scenario: null, original: null, changes: {}, dirty: false };
const byId = (id) => document.querySelector(`#${id}`);
const search = byId("search");
const layer = byId("layer");
const modelsNode = byId("models");
const examplesNode = byId("examples");
const statusNode = byId("catalog-status");
const workspaceStatus = byId("workspace-status");
const sourceSelect = byId("scenario-source");
const saveDialog = byId("save-dialog");

function appendText(parent, tag, text, className) {
  const element = document.createElement(tag);
  element.textContent = text;
  if (className) element.className = className;
  parent.append(element);
  return element;
}

async function api(path, options = {}) {
  const headers = { "X-MEHLISSA-Session": state.session, ...(options.headers || {}) };
  const response = await fetch(path, { ...options, headers, cache: "no-store" });
  const payload = await response.json().catch(() => ({}));
  if (!response.ok) {
    const error = new Error(payload.detail || "The local workbench request failed.");
    error.status = response.status;
    throw error;
  }
  return payload;
}

function normalizedQuery() { return search.value.trim().toLocaleLowerCase("en"); }
function modelMatches(model, query, selectedLayer) {
  if (selectedLayer && model.layer !== selectedLayer) return false;
  return !query || [model.id, model.layer, model.maturity, model.title].some((value) => value.toLocaleLowerCase("en").includes(query));
}
function exampleMatches(example, query, selectedLayer) {
  const related = example.model_ids.map((id) => state.models.find((model) => model.id === id)).filter(Boolean);
  if (selectedLayer && !related.some((model) => model.layer === selectedLayer)) return false;
  return !query || [example.id, example.path, example.title, ...example.model_ids].some((value) => value.toLocaleLowerCase("en").includes(query));
}

function renderModels(models) {
  modelsNode.replaceChildren(); modelsNode.setAttribute("aria-busy", "false");
  if (!models.length) appendText(modelsNode, "p", "No model families match these filters.", "empty");
  for (const model of models) {
    const card = document.createElement("article"); card.className = "model-card";
    appendText(card, "span", model.layer, "layer"); appendText(card, "h4", model.title);
    appendText(card, "p", model.id, "model-id"); appendText(card, "span", model.maturity.replaceAll("_", " "), "maturity"); modelsNode.append(card);
  }
  byId("visible-model-count").textContent = `${models.length} shown`;
}

function renderExamples(examples) {
  examplesNode.replaceChildren();
  for (const example of examples) {
    const row = document.createElement("tr"); const identity = document.createElement("td");
    identity.textContent = example.title; appendText(identity, "span", example.id, "example-id"); row.append(identity);
    const path = document.createElement("td"); appendText(path, "code", example.path); row.append(path);
    appendText(row, "td", example.model_ids.join(", ") || "General", "model-list"); examplesNode.append(row);
  }
  if (!examples.length) { const row = document.createElement("tr"); const cell = appendText(row, "td", "No starter examples match these filters.", "empty"); cell.colSpan = 3; examplesNode.append(row); }
  byId("visible-example-count").textContent = `${examples.length} shown`;
}

function applyFilters() {
  const query = normalizedQuery(); const selectedLayer = layer.value;
  renderModels(state.models.filter((model) => modelMatches(model, query, selectedLayer)));
  renderExamples(state.examples.filter((example) => exampleMatches(example, query, selectedLayer)));
}

function configureLayers() {
  for (const value of [...new Set(state.models.map((model) => model.layer))].sort()) {
    const option = document.createElement("option"); option.value = value; option.textContent = value; layer.append(option);
  }
}

function valueAtPath(documentValue, path) { return path.split(".").reduce((value, part) => value[part], documentValue); }
function setValueAtPath(documentValue, path, value) {
  const parts = path.split("."); const last = parts.pop();
  const target = parts.reduce((current, part) => current[part], documentValue); target[last] = value;
}

function updateDirtyState() {
  state.dirty = Object.keys(state.changes).length > 0;
  byId("dirty-state").textContent = state.dirty ? "Yes — not saved" : "No";
  byId("dirty-state").classList.toggle("dirty", state.dirty);
}

function updateSourceView() { byId("source-json").textContent = JSON.stringify(state.scenario.document, null, 2); }

function renderField(field) {
  const article = document.createElement("article"); article.className = `field-card${field.editable ? "" : " field-readonly"}`;
  const heading = document.createElement("div"); heading.className = "field-heading";
  const label = appendText(heading, "label", field.label); label.htmlFor = `field-${field.path.replaceAll(".", "-")}`;
  appendText(heading, "code", field.path); article.append(heading);
  let control;
  if (field.type === "boolean") {
    control = document.createElement("select");
    for (const value of ["true", "false"]) { const option = document.createElement("option"); option.value = value; option.textContent = value; control.append(option); }
    control.value = String(field.value);
  } else {
    control = document.createElement("input"); control.type = ["integer", "number"].includes(field.type) ? "number" : "text"; control.value = String(field.value);
    if (field.type === "integer") control.step = "1"; if (field.minimum !== null && field.minimum !== undefined) control.min = String(field.minimum);
    if (field.maximum !== null && field.maximum !== undefined) control.max = String(field.maximum); if (field.pattern) control.pattern = field.pattern;
  }
  control.id = `field-${field.path.replaceAll(".", "-")}`; control.disabled = !field.editable; control.required = field.required;
  control.dataset.path = field.path; control.dataset.type = field.type; article.append(control);
  appendText(article, "p", field.description, "field-description");
  const facts = document.createElement("dl"); facts.className = "field-facts";
  for (const [name, value] of [["Unit", field.unit], ["Default", field.default ?? "No automatic default"], ["Evidence", field.evidence], ["Limitation", field.limitation]]) {
    const row = document.createElement("div"); appendText(row, "dt", name); appendText(row, "dd", String(value)); facts.append(row);
  }
  article.append(facts); return article;
}

function renderEvidenceList(node, values, formatter) {
  node.replaceChildren(); for (const value of values) appendText(node, "li", formatter(value));
}

function renderScenario() {
  const scenario = state.scenario; state.original = structuredClone(scenario.document); state.changes = {}; updateDirtyState();
  byId("editor-shell").hidden = false; byId("source-kind").textContent = scenario.source.id.startsWith("template:") ? "Curated, read-only template" : "Saved workbench scenario";
  byId("source-file").textContent = scenario.source.filename; byId("field-count").textContent = String(scenario.fields.length);
  const list = byId("field-list"); list.replaceChildren(); for (const field of scenario.fields) list.append(renderField(field));
  byId("source-only-count").textContent = `(${scenario.document.artifacts?.length || 0} artifacts; ${scenario.unknown_paths.length} unknown fields)`;
  renderEvidenceList(byId("scenario-sources"), scenario.sources, (source) => `${source.citation} — ${source.role}`);
  renderEvidenceList(byId("scenario-limitations"), scenario.limitations, String); updateSourceView();
  workspaceStatus.textContent = "Scenario loaded. Guided changes remain local until you use Save as.";
}

async function loadScenario(identifier) {
  workspaceStatus.textContent = "Loading scenario…"; workspaceStatus.classList.remove("error");
  state.scenario = await api(`/api/scenario?id=${encodeURIComponent(identifier)}`); renderScenario();
}

function populateSources(selected) {
  sourceSelect.replaceChildren();
  for (const source of state.workspace.sources) { const option = document.createElement("option"); option.value = source.id; option.textContent = `${source.kind === "template" ? "Starter" : "Saved"} — ${source.title}`; sourceSelect.append(option); }
  sourceSelect.disabled = false; sourceSelect.value = selected || state.workspace.sources[0]?.id || "";
}

async function loadWorkspace() {
  state.workspace = await api("/api/scenarios");
  const modelSelect = byId("scenario-model"); modelSelect.replaceChildren(); const option = document.createElement("option"); option.value = state.workspace.model_id; option.textContent = state.workspace.model_title; modelSelect.append(option);
  populateSources(); if (!sourceSelect.value) throw new Error("No editable scenario template is available."); await loadScenario(sourceSelect.value);
}

async function loadCatalog() {
  const catalog = await api("/api/catalog");
  if (catalog.api_version !== "1.0.0") throw new Error("The workbench received an unsupported catalog response.");
  state.models = catalog.models; state.examples = catalog.examples; configureLayers();
  byId("model-count").textContent = String(state.models.length); byId("example-count").textContent = String(state.examples.length);
  statusNode.textContent = "Loaded through the local validated discovery interface."; applyFilters();
}

byId("scenario-form").addEventListener("input", (event) => {
  const control = event.target; if (!control.dataset.path) return;
  let value = control.value;
  if (control.dataset.type === "integer") value = Number.parseInt(value, 10); else if (control.dataset.type === "number") value = Number(value); else if (control.dataset.type === "boolean") value = value === "true";
  setValueAtPath(state.scenario.document, control.dataset.path, value);
  const original = valueAtPath(state.original, control.dataset.path);
  if (Object.is(value, original)) delete state.changes[control.dataset.path]; else state.changes[control.dataset.path] = value;
  updateDirtyState(); updateSourceView();
});

sourceSelect.addEventListener("change", async (event) => {
  const previous = state.scenario?.source.id;
  if (state.dirty && !window.confirm("Discard unsaved scenario changes and open another source?")) { sourceSelect.value = previous; return; }
  try { await loadScenario(event.target.value); } catch (error) { workspaceStatus.textContent = error.message; workspaceStatus.classList.add("error"); sourceSelect.value = previous; }
});

byId("reset-button").addEventListener("click", () => {
  if (!state.dirty || window.confirm("Discard all unsaved changes to this scenario?")) { state.scenario.document = structuredClone(state.original); renderScenario(); }
});
byId("save-button").addEventListener("click", () => { byId("save-error").textContent = ""; saveDialog.showModal(); byId("save-filename").focus(); });
byId("save-form").addEventListener("submit", async (event) => {
  if (event.submitter?.value === "cancel") return;
  event.preventDefault(); const filename = byId("save-filename").value; const button = byId("confirm-save"); button.disabled = true; byId("save-error").textContent = "";
  try {
    const saved = await api("/api/scenario/save", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ source_id: state.scenario.source.id, filename, changes: state.changes }) });
    state.workspace = await api("/api/scenarios"); populateSources(saved.source.id); state.scenario = saved; renderScenario(); saveDialog.close(); workspaceStatus.textContent = `Saved ${filename}. The source file was not overwritten.`; byId("save-filename").value = "";
  } catch (error) { byId("save-error").textContent = error.message; } finally { button.disabled = false; }
});

search.addEventListener("input", applyFilters); layer.addEventListener("change", applyFilters);
window.addEventListener("beforeunload", (event) => {
  if (state.dirty) { event.preventDefault(); event.returnValue = ""; }
});

async function start() {
  const parameters = new URLSearchParams(window.location.search); state.session = parameters.get("session") || ""; window.history.replaceState({}, "", window.location.pathname);
  if (!state.session) throw new Error("This workbench URL has no active local session.");
  await Promise.all([loadCatalog(), loadWorkspace()]);
}

start().catch((error) => {
  modelsNode.setAttribute("aria-busy", "false"); statusNode.textContent = error.message; statusNode.classList.add("error"); workspaceStatus.textContent = error.message; workspaceStatus.classList.add("error");
  appendText(modelsNode, "p", "Start the workbench again and use the newly printed local URL.", "empty");
});
