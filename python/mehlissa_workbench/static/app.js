// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

"use strict";

const state = { models: [], examples: [] };
const search = document.querySelector("#search");
const layer = document.querySelector("#layer");
const modelsNode = document.querySelector("#models");
const examplesNode = document.querySelector("#examples");
const statusNode = document.querySelector("#catalog-status");

function appendText(parent, tag, text, className) {
  const element = document.createElement(tag);
  element.textContent = text;
  if (className) element.className = className;
  parent.append(element);
  return element;
}

function normalizedQuery() {
  return search.value.trim().toLocaleLowerCase("en");
}

function modelMatches(model, query, selectedLayer) {
  if (selectedLayer && model.layer !== selectedLayer) return false;
  return !query || [model.id, model.layer, model.maturity, model.title]
    .some((value) => value.toLocaleLowerCase("en").includes(query));
}

function exampleMatches(example, query, selectedLayer) {
  const related = example.model_ids
    .map((id) => state.models.find((model) => model.id === id))
    .filter(Boolean);
  if (selectedLayer && !related.some((model) => model.layer === selectedLayer)) return false;
  return !query || [example.id, example.path, example.title, ...example.model_ids]
    .some((value) => value.toLocaleLowerCase("en").includes(query));
}

function renderModels(models) {
  modelsNode.replaceChildren();
  modelsNode.setAttribute("aria-busy", "false");
  if (!models.length) {
    appendText(modelsNode, "p", "No model families match these filters.", "empty");
  }
  for (const model of models) {
    const card = document.createElement("article");
    card.className = "model-card";
    appendText(card, "span", model.layer, "layer");
    appendText(card, "h4", model.title);
    appendText(card, "p", model.id, "model-id");
    appendText(card, "span", model.maturity.replaceAll("_", " "), "maturity");
    modelsNode.append(card);
  }
  document.querySelector("#visible-model-count").textContent = `${models.length} shown`;
}

function renderExamples(examples) {
  examplesNode.replaceChildren();
  for (const example of examples) {
    const row = document.createElement("tr");
    const identity = document.createElement("td");
    identity.textContent = example.title;
    appendText(identity, "span", example.id, "example-id");
    row.append(identity);
    const path = document.createElement("td");
    appendText(path, "code", example.path);
    row.append(path);
    appendText(row, "td", example.model_ids.join(", ") || "General", "model-list");
    examplesNode.append(row);
  }
  if (!examples.length) {
    const row = document.createElement("tr");
    const cell = appendText(row, "td", "No starter examples match these filters.", "empty");
    cell.colSpan = 3;
    examplesNode.append(row);
  }
  document.querySelector("#visible-example-count").textContent = `${examples.length} shown`;
}

function applyFilters() {
  const query = normalizedQuery();
  const selectedLayer = layer.value;
  renderModels(state.models.filter((model) => modelMatches(model, query, selectedLayer)));
  renderExamples(state.examples.filter((example) => exampleMatches(example, query, selectedLayer)));
}

function configureLayers() {
  const layers = [...new Set(state.models.map((model) => model.layer))].sort();
  for (const value of layers) {
    const option = document.createElement("option");
    option.value = value;
    option.textContent = value;
    layer.append(option);
  }
}

async function loadCatalog() {
  const parameters = new URLSearchParams(window.location.search);
  const session = parameters.get("session");
  window.history.replaceState({}, "", window.location.pathname);
  if (!session) throw new Error("This workbench URL has no active local session.");

  const response = await fetch("/api/catalog", {
    headers: { "X-MEHLISSA-Session": session },
    cache: "no-store",
  });
  if (!response.ok) throw new Error("The validated catalog could not be loaded.");
  const catalog = await response.json();
  if (catalog.api_version !== "1.0.0" || catalog.read_only !== true) {
    throw new Error("The workbench received an unsupported catalog response.");
  }
  state.models = catalog.models;
  state.examples = catalog.examples;
  configureLayers();
  document.querySelector("#model-count").textContent = String(state.models.length);
  document.querySelector("#example-count").textContent = String(state.examples.length);
  statusNode.textContent = "Loaded through the local validated discovery interface.";
  applyFilters();
}

search.addEventListener("input", applyFilters);
layer.addEventListener("change", applyFilters);

loadCatalog().catch((error) => {
  modelsNode.setAttribute("aria-busy", "false");
  statusNode.textContent = error.message;
  statusNode.classList.add("error");
  appendText(modelsNode, "p", "Start the workbench again and use the newly printed local URL.", "empty");
});
