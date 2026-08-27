// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_COUPLING_MODEL_COMPONENT_HPP
#define MEHLISSA_MODELS_COUPLING_MODEL_COMPONENT_HPP

#include <mehlissa/core/component_host.hpp>
#include <mehlissa/models/coupling/entity_transfer.hpp>

#include <string_view>
#include <vector>

namespace mehlissa::models::coupling {

class ModelComponent : public core::SimulationComponent {
  public:
    [[nodiscard]] virtual std::string_view model_id() const noexcept = 0;
    [[nodiscard]] virtual bool accepts_entity_at(std::string_view port_id) const noexcept = 0;
    [[nodiscard]] virtual bool emits_entity_at(std::string_view port_id) const noexcept = 0;
    virtual void accept_entity(EntityTransfer transfer) = 0;
    [[nodiscard]] virtual std::vector<EntityTransfer> take_outbound_entities() = 0;
};

} // namespace mehlissa::models::coupling

#endif // MEHLISSA_MODELS_COUPLING_MODEL_COMPONENT_HPP
