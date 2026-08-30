// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COUPLING_ENTITY_DISPOSITION_HPP
#define MEHLISSA_MODELS_COUPLING_ENTITY_DISPOSITION_HPP

#include <mehlissa/core/simulation_clock.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace mehlissa::models::coupling {

inline constexpr std::string_view entity_disposition_contract_version = "1.0.0";

enum class EntityDispositionKind : std::uint8_t { retained, adhered, extravasated };

[[nodiscard]] std::string_view to_string(EntityDispositionKind kind) noexcept;

struct EntityDispositionTransfer final {
    std::string contract_version;
    std::uint64_t entity_id{};
    std::string entity_type;
    EntityDispositionKind disposition{};
    std::string profile_id;
    std::string source_model_id;
    std::string source_port_id;
    std::string target_model_id;
    std::string target_compartment_id;
    core::SimulationClock::Duration decided_at{};
    double selection_draw{};
    double outcome_probability{};

    [[nodiscard]] bool operator==(const EntityDispositionTransfer&) const noexcept = default;
};

void validate_entity_disposition(const EntityDispositionTransfer& transfer);

class EntityDispositionSource {
  public:
    virtual ~EntityDispositionSource() = default;
    [[nodiscard]] virtual std::vector<EntityDispositionTransfer>
    take_outbound_entity_dispositions() = 0;
};

class EntityDispositionSink {
  public:
    virtual ~EntityDispositionSink() = default;
    [[nodiscard]] virtual std::string_view disposition_model_id() const noexcept = 0;
    [[nodiscard]] virtual bool
    accepts_disposition_at(std::string_view compartment_id) const noexcept = 0;
    virtual void accept_entity_disposition(EntityDispositionTransfer transfer) = 0;
};

class TerminalEntityStore final : public EntityDispositionSink {
  public:
    TerminalEntityStore(std::string model_id, std::vector<std::string> compartment_ids);

    [[nodiscard]] std::string_view disposition_model_id() const noexcept override;
    [[nodiscard]] bool
    accepts_disposition_at(std::string_view compartment_id) const noexcept override;
    void accept_entity_disposition(EntityDispositionTransfer transfer) override;

    [[nodiscard]] std::size_t resident_entity_count() const noexcept;
    [[nodiscard]] std::size_t resident_entity_count(EntityDispositionKind kind) const noexcept;
    [[nodiscard]] std::size_t resident_entity_count(std::string_view compartment_id) const noexcept;
    [[nodiscard]] bool contains_entity(std::uint64_t entity_id) const noexcept;
    [[nodiscard]] const std::vector<EntityDispositionTransfer>& entities() const noexcept;

  private:
    std::string model_id_;
    std::unordered_set<std::string> compartment_ids_;
    std::unordered_set<std::uint64_t> entity_ids_;
    std::vector<EntityDispositionTransfer> entities_;
};

} // namespace mehlissa::models::coupling

#endif // MEHLISSA_MODELS_COUPLING_ENTITY_DISPOSITION_HPP
