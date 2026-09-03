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
    const content = byId("artifact-content"); const frame = byId("report-frame");
    content.hidden = false; frame.hidden = true; content.textContent = "Loading retained evidence…"; byId("artifact-title").textContent = artifact.label; artifactDialog.showModal();
    try {
      const text = await apiText(`/api/run/artifact?id=${encodeURIComponent(job.id)}&name=${encodeURIComponent(artifact.name)}`);
      if (artifact.name === "report-html") {
        frame.srcdoc = text; frame.hidden = false; content.hidden = true;
      } else content.textContent = text;
    }
    catch (error) { byId("artifact-content").textContent = error.message; }
  });
  return button;
}

function displayValue(value) {
  if (value === null || value === undefined) return "Missing — excluded";
  if (typeof value === "boolean") return value ? "Yes" : "No";
  if (typeof value === "number") return Number.isInteger(value) ? String(value) : value.toPrecision(5).replace(/0+$/, "").replace(/\.$/, "");
  return String(value);
}

function readableName(value) { return String(value).replaceAll("_", " ").replace(/\b\w/g, (letter) => letter.toUpperCase()); }

function makeTable(headers, rows, label) {
  const frame = document.createElement("div"); frame.className = "table-frame"; frame.tabIndex = 0; frame.setAttribute("aria-label", label);
  const table = document.createElement("table"); const head = document.createElement("thead"); const headRow = document.createElement("tr");
  for (const header of headers) appendText(headRow, "th", header).scope = "col";
  head.append(headRow); table.append(head); const body = document.createElement("tbody");
  for (const row of rows) { const tableRow = document.createElement("tr"); for (const value of row) appendText(tableRow, "td", displayValue(value)); body.append(tableRow); }
  table.append(body); frame.append(table); return frame;
}

function addOutcomeCards(parent, entries) {
  const cards = document.createElement("dl"); cards.className = "outcome-cards";
  for (const [label, value] of entries) { const card = document.createElement("div"); appendText(card, "dt", label); appendText(card, "dd", displayValue(value)); cards.append(card); }
  parent.append(cards);
}

function artifactAction(jobId, name, label) {
  const job = state.jobs.find((candidate) => candidate.id === jobId);
  const artifact = job?.artifacts.find((candidate) => candidate.name === name);
  return artifact ? artifactButton(job, { ...artifact, label }) : null;
}

function appendFacts(parent, entries) {
  const facts = document.createElement("dl"); facts.className = "audit-facts";
  for (const [label, value] of entries) {
    const row = document.createElement("div"); appendText(row, "dt", label);
    appendText(row, "dd", displayValue(value)); facts.append(row);
  }
  parent.append(facts);
}

function renderSources(parent, sources) {
  const list = document.createElement("ul"); list.className = "audit-sources";
  if (!sources.length) appendText(list, "li", "No complete source declaration is available — attention required.", "audit-warning");
  for (const source of sources) {
    const item = document.createElement("li"); appendText(item, "strong", source.citation || source.id || "Unnamed source");
    if (source.role) appendText(item, "span", `Role: ${source.role}`);
    appendText(item, "span", `Licence/status: ${source.license || "Missing — attention required"}`);
    const target = source.url || source.location;
    if (target) {
      try {
        const url = new URL(target);
        if (/^https?:\/\//i.test(target) && ["http:", "https:"].includes(url.protocol)) {
          const link = appendText(item, "a", target); link.href = url.href; link.target = "_blank"; link.rel = "noopener noreferrer";
        } else appendText(item, "code", target);
      } catch { appendText(item, "code", target); }
    }
    list.append(item);
  }
  parent.append(list);
}

function downloadText(filename, mediaType, text) {
  const blob = new Blob([text], { type: mediaType });
  const url = URL.createObjectURL(blob); const link = document.createElement("a");
  link.href = url; link.download = filename; link.hidden = true; document.body.append(link); link.click(); link.remove();
  window.setTimeout(() => URL.revokeObjectURL(url), 1000);
}

function exportAudit(audit) { downloadText(`mehlissa-audit-${audit.job_id}.json`, "application/json", `${JSON.stringify(audit, null, 2)}\n`); }

const SVG_NS = "http://www.w3.org/2000/svg";
function svgElement(parent, name, attributes = {}, text = null) {
  const element = document.createElementNS(SVG_NS, name);
  for (const [key, value] of Object.entries(attributes)) element.setAttribute(key, String(value));
  if (text !== null) element.textContent = text;
  parent.append(element); return element;
}

function analysisFigure(analysis, metric, view, title, points, xLabel, description) {
  const figure = document.createElement("figure"); figure.className = "analysis-figure";
  const svg = document.createElementNS(SVG_NS, "svg"); svg.setAttribute("viewBox", "0 0 680 350"); svg.setAttribute("role", "img");
  const titleId = `chart-${analysis.job_id}-${metric.id}-${view}-title`; const descriptionId = `${titleId}-description`;
  svg.setAttribute("aria-labelledby", `${titleId} ${descriptionId}`); svgElement(svg, "title", { id: titleId }, title); svgElement(svg, "desc", { id: descriptionId }, description);
  svgElement(svg, "metadata", {}, JSON.stringify({ api_version: analysis.api_version, source_sha256: analysis.source.sha256, metric: metric.id, unit: metric.unit, view, sample_count: points.filter((point) => point.y !== null).length }));
  const left = 72; const right = 24; const top = 34; const bottom = 68; const width = 680 - left - right; const height = 350 - top - bottom;
  const included = points.filter((point) => point.y !== null); const pairView = view === "paired-difference";
  const maximumMagnitude = Math.max(0.1, ...included.map((point) => Math.abs(point.y)));
  const yMinimum = pairView ? -maximumMagnitude : 0; const yMaximum = pairView ? maximumMagnitude : 1;
  const xValues = included.map((point) => point.x); const logX = view === "sweep" && xValues.every((value) => value > 0);
  const projectedX = (value) => logX ? Math.log10(value) : value;
  const xMinimum = xValues.length ? Math.min(...xValues.map(projectedX)) : 0; const xMaximum = xValues.length ? Math.max(...xValues.map(projectedX)) : 1;
  const xPosition = (value, index) => xMinimum === xMaximum ? left + width / 2 : left + ((projectedX(value) - xMinimum) / (xMaximum - xMinimum)) * width;
  const yPosition = (value) => top + (1 - (value - yMinimum) / (yMaximum - yMinimum)) * height;
  svgElement(svg, "line", { x1: left, y1: top, x2: left, y2: top + height, class: "chart-axis" });
  svgElement(svg, "line", { x1: left, y1: top + height, x2: left + width, y2: top + height, class: "chart-axis" });
  for (let index = 0; index <= 4; index += 1) {
    const value = yMinimum + ((yMaximum - yMinimum) * index) / 4; const y = yPosition(value);
    svgElement(svg, "line", { x1: left, y1: y, x2: left + width, y2: y, class: Math.abs(value) < 1e-12 ? "chart-zero" : "chart-grid" });
    svgElement(svg, "text", { x: left - 10, y: y + 4, "text-anchor": "end", class: "chart-tick" }, displayValue(value));
  }
  included.forEach((point, index) => {
    const x = xPosition(point.x, index); const y = yPosition(point.y);
    svgElement(svg, "line", { x1: x, y1: top + height, x2: x, y2: y, class: "chart-stem" });
    svgElement(svg, "circle", { cx: x, cy: y, r: 7, class: `chart-point chart-${view}` });
    svgElement(svg, "text", { x, y: Math.max(top + 12, y - 12), "text-anchor": "middle", class: "chart-value" }, displayValue(point.y));
    svgElement(svg, "text", { x, y: top + height + 22 + (index % 2) * 13, "text-anchor": "middle", class: "chart-tick" }, point.label);
  });
  svgElement(svg, "text", { x: left + width / 2, y: 343, "text-anchor": "middle", class: "chart-label" }, xLabel);
  const yLabel = svgElement(svg, "text", { x: 17, y: top + height / 2, "text-anchor": "middle", class: "chart-label", transform: `rotate(-90 17 ${top + height / 2})` }, metric.unit); yLabel.setAttribute("aria-hidden", "true");
  figure.append(svg);
  const sampleCount = included.length; appendText(figure, "figcaption", `${title}. n=${sampleCount}; response unit: ${metric.unit}. ${description}`);
  const exportButton = document.createElement("button"); exportButton.type = "button"; exportButton.className = "secondary figure-export"; exportButton.textContent = "Download this figure SVG";
  exportButton.addEventListener("click", () => {
    const clone = svg.cloneNode(true); clone.setAttribute("xmlns", SVG_NS);
    const serialized = `<?xml version="1.0" encoding="UTF-8"?>\n${new XMLSerializer().serializeToString(clone)}\n`;
    downloadText(analysis.exports.figure_filename_pattern.replace("<metric>", metric.id).replace("<view>", view), "image/svg+xml", serialized);
  });
  figure.append(exportButton); return figure;
}

function renderAnalysisViews(section, analysis, metricId) {
  const previous = section.querySelector(".analysis-views"); if (previous) previous.remove();
  const metric = analysis.metrics.find((candidate) => candidate.id === metricId); const views = document.createElement("div"); views.className = "analysis-views";
  const replicate = metric.series.find((series) => series.design === "replicate");
  const sweep = metric.series.find((series) => series.design === "sweep");
  appendText(views, "h5", "Descriptive summaries");
  const summaries = metric.series.map((series) => {
    const summary = series.summary; const range = summary.observed_range;
    const standardDeviation = summary.sample_standard_deviation === null && summary.sample_count === 1 ? "Not defined for n<2" : summary.sample_standard_deviation;
    return [readableName(series.design), series.group, summary.sample_count, summary.missing_count, summary.mean, standardDeviation, range ? `${displayValue(range.lower)} to ${displayValue(range.upper)}` : null, "None — descriptive only"];
  });
  const pairedRange = metric.paired_summary.observed_range;
  const pairedStandardDeviation = metric.paired_summary.sample_standard_deviation === null && metric.paired_summary.sample_count === 1 ? "Not defined for n<2" : metric.paired_summary.sample_standard_deviation;
  summaries.push(["Paired difference", "comparison minus baseline", metric.paired_summary.sample_count, metric.paired_summary.missing_count, metric.paired_summary.mean, pairedStandardDeviation, pairedRange ? `${displayValue(pairedRange.lower)} to ${displayValue(pairedRange.upper)}` : null, "None — descriptive only"]);
  views.append(makeTable(["View", "Group", "n", "Missing", "Mean", "Sample SD", "Observed range (not a confidence interval)", "Inferential interval"], summaries, `${metric.label} descriptive summaries`));
  const replicatePoints = (replicate?.points || []).map((point) => ({ x: point.replicate_index, y: point.included ? point.response : null, label: `R${point.replicate_index}` }));
  const sweepPoints = (sweep?.points || []).map((point) => ({ x: point.parameter_value, y: point.included ? point.response : null, label: displayValue(point.parameter_value) }));
  const pairedPoints = metric.paired_differences.map((point, index) => ({ x: index + 1, y: point.included ? point.difference : null, label: `Seed ${point.seed}` }));
  views.append(analysisFigure(analysis, metric, "replicates", `${metric.label}: replicate observations`, replicatePoints, "Replicate", "Points show seed-to-seed observations at one fixed configuration; the reported minimum-to-maximum range is not an inferential interval."));
  views.append(analysisFigure(analysis, metric, "sweep", `${metric.label}: collector-count sweep`, sweepPoints, "Collector count (log-positioned)", "Points are deterministic parameter contrasts from the declared sweep, not uncertainty samples."));
  views.append(analysisFigure(analysis, metric, "paired-difference", `${metric.label}: paired difference`, pairedPoints, "Paired seed", "Values are comparison minus baseline within a shared seed; no population inference is made."));
  appendText(views, "h5", "Exact plotted and exported values");
  const rows = [];
  for (const series of metric.series) for (const point of series.points) rows.push([readableName(series.design), series.group, point.seed, point.parameter_value, point.response, point.included]);
  for (const point of metric.paired_differences) rows.push(["Paired difference", point.group, point.seed, "comparison - baseline", point.difference, point.included]);
  views.append(makeTable(["View", "Group", "Seed", "Parameter value", `Response (${metric.unit})`, "Included"], rows, `${metric.label} analysis values`));
  section.append(views);
}

function renderCampaignAnalysis(analysis) {
  const root = byId("dashboard-content"); const section = document.createElement("section"); section.className = "analysis-panel"; section.setAttribute("aria-labelledby", "analysis-heading");
  appendText(section, "p", "Sensitivity, observed variation, and reproducible export", "eyebrow"); const heading = appendText(section, "h4", "Campaign analysis"); heading.id = "analysis-heading";
  const boundary = appendText(section, "p", analysis.boundary.statement, "analysis-boundary"); boundary.setAttribute("role", "note");
  appendFacts(section, [["Accepted reader", analysis.reader], ["Retained observations", analysis.observation_count], ["Source SHA-256", analysis.source.sha256], ["Result schema", analysis.source.schema_version]]);
  const control = document.createElement("label"); control.className = "analysis-metric-control"; appendText(control, "span", "Response metric"); const select = document.createElement("select"); select.id = "analysis-metric";
  for (const metric of analysis.metrics) { const option = document.createElement("option"); option.value = metric.id; option.textContent = `${metric.label} (${metric.unit})`; select.append(option); } control.append(select); section.append(control);
  appendText(section, "h5", "Declared sensitivity hook");
  for (const hook of analysis.sensitivity_hooks) {
    const card = document.createElement("article"); card.className = "sensitivity-hook"; appendText(card, "strong", hook.parameter); appendText(card, "span", `Responses: ${hook.response_metrics.join(", ")}`); appendText(card, "p", hook.qualification); section.append(card);
  }
  const interpretation = document.createElement("ul"); interpretation.className = "analysis-interpretation"; for (const [label, value] of Object.entries(analysis.uncertainty)) appendText(interpretation, "li", `${readableName(label)}: ${value}`); section.append(interpretation);
  const exports = document.createElement("div"); exports.className = "artifact-actions analysis-exports";
  const jsonButton = document.createElement("button"); jsonButton.type = "button"; jsonButton.className = "secondary"; jsonButton.textContent = "Download analysis data JSON"; jsonButton.addEventListener("click", () => downloadText(analysis.exports.analysis_json_filename, "application/json", `${JSON.stringify(analysis, null, 2)}\n`)); exports.append(jsonButton);
  const csvButton = document.createElement("button"); csvButton.type = "button"; csvButton.className = "secondary"; csvButton.textContent = "Download analysis table CSV"; csvButton.addEventListener("click", () => downloadText(analysis.exports.analysis_csv_filename, "text/csv", analysis.exports.csv)); exports.append(csvButton); section.append(exports);
  root.append(section); renderAnalysisViews(section, analysis, select.value); select.addEventListener("change", () => renderAnalysisViews(section, analysis, select.value));
}

function renderAudit(audit) {
  const root = byId("dashboard-content"); const section = document.createElement("section"); section.className = "audit-panel"; section.setAttribute("aria-labelledby", "audit-heading");
  appendText(section, "p", "Provenance, evidence, and interpretation", "eyebrow"); const heading = appendText(section, "h4", "Audit summary"); heading.id = "audit-heading";
  const boundary = appendText(section, "p", audit.boundary.statement, "audit-boundary"); boundary.setAttribute("role", "note");
  const statusRow = document.createElement("div"); statusRow.className = "audit-status-row";
  appendText(statusRow, "span", `Integrity: ${readableName(audit.integrity.status)}`, `audit-state ${audit.integrity.status}`);
  appendText(statusRow, "span", `Evidence: ${readableName(audit.evidence.status)}`, `audit-state ${audit.evidence.status}`); section.append(statusRow);
  appendFacts(section, [
    ["Run", audit.run.title], ["Completed", audit.run.completed_at], ["Seeds", audit.run.master_seeds.join(", ")],
    ["Maturity", audit.interpretation.maturity], ["Acceptance level", audit.interpretation.acceptance_level || "Not applicable"],
    ["Interpretation", audit.interpretation.claim],
  ]);
  if (audit.workbench && Object.keys(audit.workbench).length) {
    appendText(section, "h5", "Workbench audit producer");
    appendFacts(section, [["Application", audit.workbench.name], ["Workbench version", audit.workbench.version], ["Audit contract", audit.workbench.audit_api_version]]);
  }
  if (audit.software && Object.keys(audit.software).length) {
    appendText(section, "h5", "Software and build");
    appendFacts(section, [["MEHLISSA", audit.software.version], ["Git commit", audit.software.git_commit], ["Git state", audit.software.git_dirty ? "Dirty working tree recorded" : "Clean working tree recorded"], ["Compiler", `${audit.software.compiler_id || "Unknown"} ${audit.software.compiler_version || ""}`.trim()], ["Platform", `${audit.software.operating_system || "Unknown"} / ${audit.software.architecture || "Unknown"}`]]);
  }
  appendText(section, "h5", `Integrity checks (${audit.integrity.verified_count}/${audit.integrity.checked_count} verified)`);
  section.append(makeTable(["Status", "Item", "Expected SHA-256", "Actual SHA-256", "Retained path"], audit.integrity.checks.map((check) => [readableName(check.status), check.label, check.expected_sha256, check.actual_sha256, check.path]), "Provenance integrity checks"));
  if (audit.derived_runs?.length) {
    appendText(section, "h5", "Derived-run provenance");
    section.append(makeTable(["Run", "Seed", "Manifest", "Result", "MEHLISSA", "Git commit"], audit.derived_runs.map((run) => [run.id, run.seed, run.manifest.status, run.result.status, run.software.version, run.software.git_commit]), "Campaign derived-run provenance"));
  }
  appendText(section, "h5", "Scenario evidence and licences"); renderSources(section, audit.evidence.scenario_sources);
  if (audit.evidence.components.length) {
    appendText(section, "h5", "Model and input maturity"); const components = document.createElement("div"); components.className = "component-audits";
    for (const component of audit.evidence.components) {
      const details = document.createElement("details"); details.className = component.evidence_complete ? "component-audit" : "component-audit audit-attention";
      const title = component.title || component.id || component.definition_path;
      appendText(details, "summary", `${readableName(component.role)} — ${title} — ${component.maturity.label}`);
      appendFacts(details, [["Model/profile", component.id], ["Version", component.version], ["Evidence class", component.maturity.evidence_class], ["Clinical maturity", component.maturity.clinical_maturity], ["Definition integrity", component.integrity.definition], ["Schema integrity", component.integrity.schema], ["Definition", component.definition_path], ["Schema", component.schema_path]]);
      renderSources(details, component.sources);
      if (component.limitations.length) { appendText(details, "h6", "Limitations"); const list = document.createElement("ul"); for (const item of component.limitations) appendText(list, "li", item); details.append(list); }
      components.append(details);
    }
    section.append(components);
  }
  appendText(section, "h5", "Interpretation limitations"); const limitations = document.createElement("ul"); limitations.className = "dashboard-limitations";
  const uniqueLimitations = new Set(); for (const group of Object.values(audit.limitations)) if (Array.isArray(group)) for (const item of group) uniqueLimitations.add(item);
  for (const item of uniqueLimitations) appendText(limitations, "li", item); section.append(limitations);
  const actions = document.createElement("div"); actions.className = "artifact-actions"; const download = document.createElement("button"); download.type = "button"; download.className = "secondary"; download.textContent = "Download complete audit JSON"; download.addEventListener("click", () => exportAudit(audit)); actions.append(download);
  const raw = document.createElement("details"); raw.className = "raw-audit"; appendText(raw, "summary", "Complete machine-readable audit and original provenance"); const pre = appendText(raw, "pre", JSON.stringify(audit, null, 2)); pre.tabIndex = 0; actions.append(raw); section.append(actions); root.append(section);
}

function renderScenarioDashboard(data) {
  const root = byId("dashboard-content"); const summary = data.summary;
  appendText(root, "p", `Accepted by ${data.reader}; one completed run is included.`, "reader-note");
  addOutcomeCards(root, [["Detected", summary.detected], ["Assembled", summary.assembled], ["Sensitivity", summary.sensitivity], ["Specificity", summary.specificity], ["Collectors", summary.collector_count], ["Seed", summary.seed]]);
  appendText(root, "h4", "Cumulative stage timing");
  root.append(makeTable(["Stage", "Time (ms)", "Basis"], data.runtime_stages.map((stage) => [readableName(stage.stage), stage.time_ms, readableName(stage.basis)]), "Scenario stage timing"));
  appendText(root, "h4", "Analysis cases");
  root.append(makeTable(["Case", "Target present", "Detected", "Classification", "Bound fraction"], data.analysis_cases.map((item) => [item.case_id, item.target_present, item.detected, readableName(item.classification), item.final_bound_fraction]), "Scenario analysis cases"));
  const actions = document.createElement("div"); actions.className = "artifact-actions";
  for (const action of [artifactAction(data.job_id, data.authoritative_artifact, "Open authoritative result JSON"), artifactAction(data.job_id, data.ux3_report_artifact, "Open UX-3 HTML report")]) if (action) actions.append(action);
  root.append(actions);
  appendText(root, "p", "Sensitivity and uncertainty views require a completed campaign. One scenario is not treated as an uncertainty sample.", "excluded-result");
}

function renderCampaignDashboard(data) {
  const root = byId("dashboard-content");
  appendText(root, "p", `Accepted by ${data.reader}; ${data.observation_count} completed derived runs are included.`, "reader-note");
  addOutcomeCards(root, [["Completed runs", data.summary.run_count], ["Experimental groups", data.summary.group_count], ["Campaign", data.summary.campaign_id]]);
  for (const group of data.groups) {
    appendText(root, "h4", `${readableName(group.name)} — ${readableName(group.design)} (${group.run_count})`);
    root.append(makeTable(["Run", "Role", "Replicate", "Seed", "Collectors", "Detected", "Assembled", "Sensitivity", "Specificity"], group.runs.map((run) => [run.id, readableName(run.role), run.replicate_index, run.seed, run.value, run.detected, run.assembled, run.sensitivity, run.specificity]), `${group.name} campaign runs`));
  }
  appendText(root, "h4", "Paired differences (comparison minus baseline)");
  root.append(makeTable(["Metric", "Seed", "Baseline", "Comparison", "Difference", "Included"], data.paired_differences.map((row) => [readableName(row.metric), row.seed, row.baseline, row.comparison, row.difference, row.included]), "Campaign paired differences"));
  const limitations = document.createElement("ul"); limitations.className = "dashboard-limitations"; for (const item of data.limitations) appendText(limitations, "li", item); root.append(limitations);
  const actions = document.createElement("div"); actions.className = "artifact-actions";
  for (const action of [artifactAction(data.job_id, data.authoritative_artifact, "Open authoritative campaign JSON"), artifactAction(data.job_id, data.table_artifact, "Open campaign CSV")]) if (action) actions.append(action);
  root.append(actions);
}

async function openDashboard(jobId = byId("dashboard-run").value) {
  const root = byId("dashboard-content"); root.replaceChildren(); byId("results-status").textContent = "Reading retained results…";
  try {
    const data = await api(`/api/run/dashboard?id=${encodeURIComponent(jobId)}`);
    if (!data.available) {
      appendText(root, "p", data.reason, "excluded-result");
      addOutcomeCards(root, [["Job status", data.status], ["Included observations", data.observation_count]]);
      byId("results-status").textContent = "No result values are available for this job."; return;
    }
    if (data.kind === "scenario") renderScenarioDashboard(data); else renderCampaignDashboard(data);
    if (data.kind === "campaign") {
      try {
        const analysis = await api(`/api/run/analysis?id=${encodeURIComponent(jobId)}`); renderCampaignAnalysis(analysis);
      } catch (error) {
        appendText(root, "p", `The retained result is visible, but campaign analysis could not be completed: ${error.message}`, "excluded-result");
      }
    }
    try {
      const audit = await api(`/api/run/audit?id=${encodeURIComponent(jobId)}`); renderAudit(audit);
      byId("results-status").textContent = `${readableName(data.kind)} dashboard${data.kind === "campaign" ? ", descriptive analysis," : ""} and provenance audit loaded.`;
    } catch (error) {
      appendText(root, "p", `The result is visible, but its audit could not be completed: ${error.message}`, "excluded-result");
      byId("results-status").textContent = "Result loaded; provenance audit needs attention.";
    }
  } catch (error) { appendText(root, "p", error.message, "excluded-result"); byId("results-status").textContent = "The retained result could not be read."; }
}

function populateResultControls() {
  const dashboard = byId("dashboard-run"); const selected = dashboard.value; dashboard.replaceChildren();
  if (!state.jobs.length) { const option = document.createElement("option"); option.value = ""; option.textContent = "No runs in this session"; dashboard.append(option); }
  for (const job of state.jobs) { const option = document.createElement("option"); option.value = job.id; option.textContent = `${job.kind} — ${job.title} — ${job.status}`; dashboard.append(option); }
  dashboard.value = state.jobs.some((job) => job.id === selected) ? selected : (state.jobs.find((job) => job.status === "completed")?.id || state.jobs[0]?.id || "");
  byId("open-dashboard").disabled = !dashboard.value;

  const completed = state.jobs.filter((job) => job.kind === "scenario" && job.status === "completed");
  for (const id of ["compare-left", "compare-right"]) {
    const select = byId(id); const previous = select.value; select.replaceChildren();
    for (const job of completed) { const option = document.createElement("option"); option.value = job.id; option.textContent = `${job.title} — ${job.id}`; select.append(option); }
    if (completed.some((job) => job.id === previous)) select.value = previous; select.disabled = completed.length < 2;
  }
  if (completed.length >= 2 && byId("compare-left").value === byId("compare-right").value) {
    byId("compare-right").value = completed.find((job) => job.id !== byId("compare-left").value).id;
  }
  byId("compare-runs").disabled = completed.length < 2;
}

function renderJob(job) {
  const card = document.createElement("article"); card.className = `run-card run-${job.status}`;
  const heading = document.createElement("div"); heading.className = "run-card-heading"; const title = document.createElement("div");
  appendText(title, "span", job.kind, "run-kind"); appendText(title, "h4", job.title); heading.append(title); appendText(heading, "strong", job.status, "run-state"); card.append(heading);
  appendText(card, "p", job.stage, "run-stage"); const progress = document.createElement("progress"); progress.max = 100; progress.value = job.progress_percent; progress.setAttribute("aria-label", `Run progress: ${job.progress_percent}%`); card.append(progress);
  const facts = document.createElement("dl"); facts.className = "run-facts"; addPlanRow(facts, "Run ID", job.id); addPlanRow(facts, "Output", job.directory); addPlanRow(facts, "Runs", String(job.plan.run_count)); addPlanRow(facts, "Seeds", job.plan.master_seeds.join(", ")); card.append(facts);
  if (job.error) appendText(card, "p", job.error, "run-error");
  const actions = document.createElement("div"); actions.className = "artifact-actions"; for (const artifact of job.artifacts) actions.append(artifactButton(job, artifact)); card.append(actions);
  const dashboard = document.createElement("button"); dashboard.type = "button"; dashboard.className = "secondary dashboard-link"; dashboard.textContent = job.status === "completed" ? "View result dashboard" : "Explain result availability";
  dashboard.addEventListener("click", async () => { byId("dashboard-run").value = job.id; await openDashboard(job.id); byId("results-title").scrollIntoView({ behavior: "smooth" }); }); card.append(dashboard);
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
  populateResultControls();
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
byId("open-dashboard").addEventListener("click", () => openDashboard());
byId("comparison-form").addEventListener("submit", async (event) => {
  event.preventDefault(); const root = byId("comparison-content"); root.replaceChildren(); appendText(root, "p", "Comparing accepted results…", "empty");
  try {
    const data = await api("/api/run/compare", { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify({ left_id: byId("compare-left").value, right_id: byId("compare-right").value }) });
    root.replaceChildren(); appendText(root, "p", `${data.left.run_id} compared with ${data.right.run_id}. ${data.interpretation}`, "reader-note");
    root.append(makeTable(["Metric", "Left", "Right", "Difference"], data.rows.map((row) => [readableName(row.metric), row.left, row.right, !row.comparable ? "Missing — excluded" : (row.difference === null ? "Not numeric" : row.difference)]), "Side-by-side scenario comparison"));
  } catch (error) { root.replaceChildren(); appendText(root, "p", error.message, "excluded-result"); }
});
artifactDialog.addEventListener("close", () => { byId("report-frame").srcdoc = ""; });
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
