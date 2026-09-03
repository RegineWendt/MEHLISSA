// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

"use strict";

const state = { models: [], examples: [], session: "", workspace: null, scenario: null, original: null, changes: {}, dirty: false, validation: null, validationTimer: null, validationRequest: 0, fieldCards: new Map(), runPlans: null, jobs: [], pendingRun: null, runPoll: null };
const byId = (id) => document.querySelector(`#${id}`);
const search = byId("search");
const layer = byId("layer");
const modelsNode = byId("models");
const examplesNode = byId("examples");
const statusNode = byId("catalog-status");
const workspaceStatus = byId("workspace-status");
const sourceSelect = byId("scenario-source");
const saveDialog = byId("save-dialog");
const runDialog = byId("run-dialog");
const artifactDialog = byId("artifact-dialog");

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
    error.status = response.status; error.payload = payload;
    throw error;
  }
  return payload;
}

async function apiText(path) {
  const response = await fetch(path, { headers: { "X-MEHLISSA-Session": state.session }, cache: "no-store" });
  if (!response.ok) throw new Error("The retained artifact is unavailable.");
  return response.text();
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

function setValidationPending() {
  state.validation = null;
  byId("validation-state").textContent = "Checking…";
  byId("validation-state").className = "validation-state pending";
  byId("validation-counts").textContent = "Authoritative validation is running.";
  byId("validation-issues").replaceChildren();
  byId("validation-summary").textContent = "Validation summary will appear here.";
  byId("copy-validation").disabled = true;
  byId("save-button").disabled = true;
  byId("execution-gate").textContent = "Closed while validation is pending";
  updateRunAvailability();
}

function renderValidation(report) {
  state.validation = report;
  for (const card of state.fieldCards.values()) {
    card.article.classList.remove("field-error", "field-warning");
    card.message.replaceChildren();
  }
  const issuesNode = byId("validation-issues"); issuesNode.replaceChildren();
  for (const issue of report.issues) {
    const item = document.createElement("li"); item.className = `validation-issue ${issue.severity}`;
    appendText(item, "strong", `${issue.code} · ${issue.path}`);
    appendText(item, "span", issue.message);
    appendText(item, "span", `How to repair: ${issue.guidance}`, "repair");
    issuesNode.append(item);
    const field = state.fieldCards.get(issue.path);
    if (field) {
      field.article.classList.add(issue.severity === "error" ? "field-error" : "field-warning");
      appendText(field.message, "span", `${issue.code}: ${issue.message} ${issue.guidance}`);
    }
  }
  if (!report.issues.length) appendText(issuesNode, "li", "No errors or warnings were found.", "validation-empty");
  const status = byId("validation-state");
  status.textContent = report.valid ? "Valid" : "Invalid";
  status.className = `validation-state ${report.valid ? "valid" : "invalid"}`;
  byId("validation-counts").textContent = `${report.error_count} errors · ${report.warning_count} warnings · accepted CLI decision`;
  byId("validation-summary").textContent = report.summary_text;
  byId("copy-validation").disabled = false;
  byId("save-button").disabled = !report.valid;
  byId("execution-gate").textContent = report.run_allowed ? "Open — current candidate can be reviewed and run" : "Closed — invalid scenarios cannot run";
  updateRunAvailability();
  if (state.runPlans) renderRunPlan();
}

async function validateScenario(requestNumber = ++state.validationRequest) {
  setValidationPending();
  try {
    const report = await api("/api/scenario/validate", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ source_id: state.scenario.source.id, changes: state.changes }) });
    if (requestNumber === state.validationRequest) renderValidation(report);
  } catch (error) {
    if (requestNumber !== state.validationRequest) return;
    byId("validation-state").textContent = "Unavailable"; byId("validation-state").className = "validation-state invalid";
    byId("validation-counts").textContent = error.message; byId("execution-gate").textContent = "Closed — validation unavailable";
  }
}

function scheduleValidation() {
  window.clearTimeout(state.validationTimer); const requestNumber = ++state.validationRequest; setValidationPending();
  state.validationTimer = window.setTimeout(() => validateScenario(requestNumber), 350);
}

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
  control.dataset.path = field.path; control.dataset.type = field.type; article.dataset.fieldPath = field.path; article.append(control);
  appendText(article, "p", field.description, "field-description");
  const validation = appendText(article, "p", "", "field-validation"); validation.id = `${control.id}-validation`; validation.setAttribute("role", "status");
  control.setAttribute("aria-describedby", validation.id);
  const facts = document.createElement("dl"); facts.className = "field-facts";
  for (const [name, value] of [["Unit", field.unit], ["Default", field.default ?? "No automatic default"], ["Evidence", field.evidence], ["Limitation", field.limitation]]) {
    const row = document.createElement("div"); appendText(row, "dt", name); appendText(row, "dd", String(value)); facts.append(row);
  }
  article.append(facts); state.fieldCards.set(field.path, { article, message: validation }); return article;
}

function renderEvidenceList(node, values, formatter) {
  node.replaceChildren(); for (const value of values) appendText(node, "li", formatter(value));
}

function renderScenario() {
  const scenario = state.scenario; state.original = structuredClone(scenario.document); state.changes = {}; state.fieldCards.clear(); updateDirtyState();
  byId("editor-shell").hidden = false; byId("source-kind").textContent = scenario.source.id.startsWith("template:") ? "Curated, read-only template" : "Saved workbench scenario";
  byId("source-file").textContent = scenario.source.filename; byId("field-count").textContent = String(scenario.fields.length);
  const list = byId("field-list"); list.replaceChildren(); for (const field of scenario.fields) list.append(renderField(field));
  byId("source-only-count").textContent = `(${scenario.document.artifacts?.length || 0} artifacts; ${scenario.unknown_paths.length} unknown fields)`;
  renderEvidenceList(byId("scenario-sources"), scenario.sources, (source) => `${source.citation} — ${source.role}`);
  renderEvidenceList(byId("scenario-limitations"), scenario.limitations, String); updateSourceView();
  workspaceStatus.textContent = "Scenario loaded. Validation now checks the complete candidate."; validateScenario();
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

function updateRunAvailability() {
  const button = byId("review-run");
  if (!button) return;
  const scenarioSelected = byId("run-kind").value === "scenario";
  button.disabled = !state.runPlans || (scenarioSelected && !state.validation?.run_allowed);
}

function addPlanRow(list, term, description) {
  const row = document.createElement("div"); appendText(row, "dt", term); appendText(row, "dd", description); list.append(row);
}

function selectedCampaign() { return state.runPlans?.campaigns.find((campaign) => campaign.id === byId("campaign-select").value); }

function renderRunPlan() {
  if (!state.runPlans) return;
  const campaignMode = byId("run-kind").value === "campaign";
  byId("campaign-choice").hidden = !campaignMode;
  const plan = byId("run-plan"); const limitations = byId("run-limitations"); plan.replaceChildren(); limitations.replaceChildren();
  if (campaignMode) {
    const campaign = selectedCampaign();
    addPlanRow(plan, "Plan", campaign.title); addPlanRow(plan, "Derived runs", String(campaign.run_count));
    addPlanRow(plan, "Replicates", `${campaign.replicates.count} runs from seed ${campaign.replicates.first_seed}`);
    addPlanRow(plan, "Sweep", campaign.sweeps.map((sweep) => `${sweep.parameter}: ${sweep.values.join(" vs ")}`).join("; "));
    addPlanRow(plan, "Paired comparison", campaign.paired_comparisons.map((pair) => `${pair.baseline} vs ${pair.comparison}, shared seed ${pair.first_seed}`).join("; "));
    addPlanRow(plan, "Manifest", campaign.manifest); addPlanRow(plan, "Manifest SHA-256", campaign.manifest_sha256); for (const item of campaign.limitations) appendText(limitations, "li", item);
  } else {
    addPlanRow(plan, "Scenario", state.scenario?.source.title || "Loading…");
    addPlanRow(plan, "Runs", "1 validated scenario"); addPlanRow(plan, "Candidate", state.validation?.candidate_sha256 || "Validation pending");
    addPlanRow(plan, "Master seed", String(state.scenario?.document.run?.master_seed ?? "—"));
    appendText(limitations, "li", "The run preserves the scenario's evidence statements and interpretation limitations.");
  }
  updateRunAvailability();
}

function artifactButton(job, artifact) {
  const button = document.createElement("button"); button.type = "button"; button.className = "artifact-link"; button.textContent = artifact.label;
  button.addEventListener("click", async () => {
    byId("artifact-title").textContent = artifact.label; byId("artifact-content").textContent = "Loading retained evidence…"; artifactDialog.showModal();
    try { byId("artifact-content").textContent = await apiText(`/api/run/artifact?id=${encodeURIComponent(job.id)}&name=${encodeURIComponent(artifact.name)}`); }
    catch (error) { byId("artifact-content").textContent = error.message; }
  });
  return button;
}

function renderJob(job) {
  const card = document.createElement("article"); card.className = `run-card run-${job.status}`;
  const heading = document.createElement("div"); heading.className = "run-card-heading"; const title = document.createElement("div");
  appendText(title, "span", job.kind, "run-kind"); appendText(title, "h4", job.title); heading.append(title); appendText(heading, "strong", job.status, "run-state"); card.append(heading);
  appendText(card, "p", job.stage, "run-stage"); const progress = document.createElement("progress"); progress.max = 100; progress.value = job.progress_percent; progress.setAttribute("aria-label", `Run progress: ${job.progress_percent}%`); card.append(progress);
  const facts = document.createElement("dl"); facts.className = "run-facts"; addPlanRow(facts, "Run ID", job.id); addPlanRow(facts, "Output", job.directory); addPlanRow(facts, "Runs", String(job.plan.run_count)); addPlanRow(facts, "Seeds", job.plan.master_seeds.join(", ")); card.append(facts);
  if (job.error) appendText(card, "p", job.error, "run-error");
  const actions = document.createElement("div"); actions.className = "artifact-actions"; for (const artifact of job.artifacts) actions.append(artifactButton(job, artifact)); card.append(actions);
  const details = document.createElement("details"); appendText(details, "summary", `Bounded log (${job.logs.length}/${200} lines maximum)`); const log = appendText(details, "pre", job.logs.join("\n")); log.tabIndex = 0; card.append(details);
  if (["queued", "running"].includes(job.status)) {
    const cancel = document.createElement("button"); cancel.type = "button"; cancel.className = "danger"; cancel.textContent = job.cancel_requested ? "Cancellation requested…" : "Cancel run"; cancel.disabled = job.cancel_requested;
    cancel.addEventListener("click", async () => { if (!window.confirm("Cancel this run? Inputs, logs, partial outputs, and the cancellation state will be retained.")) return; await api("/api/run/cancel", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ id: job.id }) }); await loadRuns(); }); card.append(cancel);
  }
  return card;
}

function renderRuns() {
  const list = byId("run-list"); list.replaceChildren();
  if (!state.jobs.length) appendText(list, "p", "No workbench runs in this session.", "empty");
  for (const job of state.jobs) list.append(renderJob(job));
  const active = state.jobs.some((job) => ["queued", "running", "collecting"].includes(job.status));
  window.clearTimeout(state.runPoll); if (active) state.runPoll = window.setTimeout(loadRuns, 750);
}

async function loadRuns() {
  const response = await api("/api/runs"); state.jobs = response.jobs; renderRuns();
  byId("run-status").textContent = state.jobs.length ? `${state.jobs.length} retained run record${state.jobs.length === 1 ? "" : "s"} in this session.` : "Ready. No run has been started in this session.";
}

async function loadRunPlans() {
  state.runPlans = await api("/api/run-plans"); byId("output-root").textContent = state.runPlans.output_root;
  const select = byId("campaign-select"); select.replaceChildren(); for (const campaign of state.runPlans.campaigns) { const option = document.createElement("option"); option.value = campaign.id; option.textContent = campaign.title; select.append(option); }
  renderRunPlan(); await loadRuns();
}

function runSummary() {
  const campaignMode = byId("run-kind").value === "campaign"; const label = byId("output-label").value;
  if (campaignMode) { const campaign = selectedCampaign(); return `Type: controlled campaign\nPlan: ${campaign.title}\nManifest SHA-256: ${campaign.manifest_sha256}\nDerived runs: ${campaign.run_count}\nReplicates: ${campaign.replicates.count}\nSweep: ${campaign.sweeps[0].values.join(" vs ")} collectors\nPaired comparison: ${campaign.paired_comparisons[0].baseline} vs ${campaign.paired_comparisons[0].comparison}\nOutput: ${state.runPlans.output_root}/${label}-<unique-id>`; }
  return `Type: individual scenario\nScenario: ${state.scenario.source.title}\nCandidate SHA-256: ${state.validation.candidate_sha256}\nMaster seed: ${state.scenario.document.run.master_seed}\nOutput: ${state.runPlans.output_root}/${label}-<unique-id>`;
}

byId("scenario-form").addEventListener("input", (event) => {
  const control = event.target; if (!control.dataset.path) return;
  let value = control.value;
  if (control.dataset.type === "integer") value = Number.parseInt(value, 10); else if (control.dataset.type === "number") value = Number(value); else if (control.dataset.type === "boolean") value = value === "true";
  setValueAtPath(state.scenario.document, control.dataset.path, value);
  const original = valueAtPath(state.original, control.dataset.path);
  if (Object.is(value, original)) delete state.changes[control.dataset.path]; else state.changes[control.dataset.path] = value;
  updateDirtyState(); updateSourceView();
  scheduleValidation();
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
byId("copy-validation").addEventListener("click", async () => {
  if (!state.validation) return;
  try { await navigator.clipboard.writeText(state.validation.summary_text); workspaceStatus.textContent = "Validation summary copied to the clipboard."; }
  catch { workspaceStatus.textContent = "Clipboard access was denied. Select the summary text and copy it manually."; }
});
byId("save-form").addEventListener("submit", async (event) => {
  if (event.submitter?.value === "cancel") return;
  event.preventDefault(); const filename = byId("save-filename").value; const button = byId("confirm-save"); button.disabled = true; byId("save-error").textContent = "";
  try {
    const saved = await api("/api/scenario/save", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ source_id: state.scenario.source.id, filename, changes: state.changes }) });
    state.workspace = await api("/api/scenarios"); populateSources(saved.source.id); state.scenario = saved; renderScenario(); saveDialog.close(); workspaceStatus.textContent = `Saved ${filename}. The source file was not overwritten.`; byId("save-filename").value = "";
  } catch (error) {
    byId("save-error").textContent = error.message;
    if (error.payload?.validation) renderValidation(error.payload.validation);
  } finally { button.disabled = false; }
});

byId("run-kind").addEventListener("change", renderRunPlan);
byId("campaign-select").addEventListener("change", renderRunPlan);
byId("refresh-runs").addEventListener("click", () => loadRuns().catch((error) => { byId("run-status").textContent = error.message; }));
byId("run-form").addEventListener("submit", (event) => {
  event.preventDefault();
  if (byId("run-kind").value === "scenario" && !state.validation?.run_allowed) return;
  state.pendingRun = { kind: byId("run-kind").value, outputLabel: byId("output-label").value };
  byId("confirm-run-summary").textContent = runSummary(); byId("run-confirmed").checked = false; byId("run-error").textContent = ""; runDialog.showModal();
});
byId("confirm-run-form").addEventListener("submit", async (event) => {
  if (event.submitter?.value === "cancel") return;
  event.preventDefault(); if (!byId("run-confirmed").checked || !state.pendingRun) return;
  const button = byId("confirm-run"); button.disabled = true; byId("run-error").textContent = "";
  try {
    const campaignMode = state.pendingRun.kind === "campaign";
    const path = campaignMode ? "/api/run/campaign" : "/api/run/scenario";
    const body = campaignMode
      ? { campaign_id: byId("campaign-select").value, output_label: state.pendingRun.outputLabel, confirmed: true }
      : { source_id: state.scenario.source.id, changes: state.changes, output_label: state.pendingRun.outputLabel, confirmed: true };
    await api(path, { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(body) });
    runDialog.close(); byId("run-status").textContent = "Run started. Monitoring retained evidence…"; await loadRuns();
  } catch (error) {
    byId("run-error").textContent = error.message; if (error.payload?.validation) renderValidation(error.payload.validation);
  } finally { button.disabled = false; }
});

search.addEventListener("input", applyFilters); layer.addEventListener("change", applyFilters);
window.addEventListener("beforeunload", (event) => {
  if (state.dirty) { event.preventDefault(); event.returnValue = ""; }
});

async function start() {
  const parameters = new URLSearchParams(window.location.search); state.session = parameters.get("session") || ""; window.history.replaceState({}, "", window.location.pathname);
  if (!state.session) throw new Error("This workbench URL has no active local session.");
  await Promise.all([loadCatalog(), loadWorkspace(), loadRunPlans()]);
}

start().catch((error) => {
  modelsNode.setAttribute("aria-busy", "false"); statusNode.textContent = error.message; statusNode.classList.add("error"); workspaceStatus.textContent = error.message; workspaceStatus.classList.add("error");
  appendText(modelsNode, "p", "Start the workbench again and use the newly printed local URL.", "empty");
});
