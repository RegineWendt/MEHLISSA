// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include <mehlissa/models/coupling/entity_disposition.hpp>

#include <mehlissa/core/error.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace mehlissa::models::coupling {
namespace {

[[nodiscard]] bool valid_kind(const EntityDispositionKind kind) noexcept {
    switch (kind) {
    case EntityDispositionKind::retained:
    case EntityDispositionKind::adhered:
    case EntityDispositionKind::extravasated:
        return true;
    }
    return false;
}

} // namespace

std::string_view to_string(const EntityDispositionKind kind) noexcept {
    switch (kind) {
    case EntityDispositionKind::retained:
        return "retained";
    case EntityDispositionKind::adhered:
        return "adhered";
    case EntityDispositionKind::extravasated:
        return "extravasated";
    }
    return "unknown";
}

void validate_entity_disposition(const EntityDispositionTransfer& transfer) {
    if (transfer.contract_version != entity_disposition_contract_version) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Unsupported entity-disposition contract version: " +
                                      transfer.contract_version};
    }
    if (transfer.entity_id == 0 || transfer.entity_type.empty() ||
        !valid_kind(transfer.disposition) || transfer.profile_id.empty() ||
        transfer.source_model_id.empty() || transfer.source_port_id.empty() ||
        transfer.target_model_id.empty() || transfer.target_compartment_id.empty()) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "An entity disposition requires an identity, profile, source, and target owner"};
    }
    if (transfer.decided_at < core::SimulationClock::Duration::zero() ||
        !std::isfinite(transfer.selection_draw) || transfer.selection_draw <= 0.0 ||
        transfer.selection_draw >= 1.0 || !std::isfinite(transfer.outcome_probability) ||
        transfer.outcome_probability <= 0.0 || transfer.outcome_probability > 1.0) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "An entity disposition requires valid time, selection draw, and outcome probability"};
    }
}

TerminalEntityStore::TerminalEntityStore(std::string model_id,
                                         std::vector<std::string> compartment_ids)
    : model_id_{std::move(model_id)},
      compartment_ids_{compartment_ids.begin(), compartment_ids.end()} {
    if (model_id_.empty() || compartment_ids.empty() ||
        compartment_ids_.size() != compartment_ids.size() ||
        std::ranges::any_of(compartment_ids, [](const auto& id) { return id.empty(); })) {
        throw core::MehlissaError{
            core::ErrorCode::data_invalid,
            "A terminal entity store requires one model and unique non-empty compartments"};
    }
}

std::string_view TerminalEntityStore::disposition_model_id() const noexcept { return model_id_; }

bool TerminalEntityStore::accepts_disposition_at(
    const std::string_view compartment_id) const noexcept {
    return std::ranges::any_of(compartment_ids_, [compartment_id](const auto& candidate) {
        return candidate == compartment_id;
    });
}

void TerminalEntityStore::accept_entity_disposition(EntityDispositionTransfer transfer) {
    validate_entity_disposition(transfer);
    if (transfer.target_model_id != model_id_ ||
        !accepts_disposition_at(transfer.target_compartment_id)) {
        throw core::MehlissaError{core::ErrorCode::data_invalid,
                                  "Entity disposition does not target this terminal store"};
    }
    if (entity_ids_.contains(transfer.entity_id)) {
        throw core::MehlissaError{core::ErrorCode::invariant_violated,
                                  "Terminal store already owns this entity ID"};
    }
    entities_.push_back(std::move(transfer));
    try {
        entity_ids_.insert(entities_.back().entity_id);
    } catch (...) {
        entities_.pop_back();
        throw;
    }
}

std::size_t TerminalEntityStore::resident_entity_count() const noexcept { return entities_.size(); }

std::size_t
TerminalEntityStore::resident_entity_count(const EntityDispositionKind kind) const noexcept {
    return static_cast<std::size_t>(
        std::ranges::count(entities_, kind, &EntityDispositionTransfer::disposition));
}

std::size_t
TerminalEntityStore::resident_entity_count(const std::string_view compartment_id) const noexcept {
    return static_cast<std::size_t>(std::ranges::count(
        entities_, compartment_id, &EntityDispositionTransfer::target_compartment_id));
}

bool TerminalEntityStore::contains_entity(const std::uint64_t entity_id) const noexcept {
    return entity_ids_.contains(entity_id);
}

const std::vector<EntityDispositionTransfer>& TerminalEntityStore::entities() const noexcept {
    return entities_;
}

} // namespace mehlissa::models::coupling
