// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_ANALYTICAL_RECEPTOR_LIGAND_MODEL_HPP
#define MEHLISSA_MODELS_CELL_ANALYTICAL_RECEPTOR_LIGAND_MODEL_HPP

#include <mehlissa/models/cell/receptor_ligand_model.hpp>

namespace mehlissa::models::cell {

inline constexpr auto analytical_receptor_ligand_kind =
    "analytical_receptor_ligand_constant_reservoir";

class AnalyticalReceptorLigandModel final : public ReceptorLigandModel {
  public:
    explicit AnalyticalReceptorLigandModel(ReceptorLigandModelConfig config);

    [[nodiscard]] std::string_view kind() const noexcept override;
    [[nodiscard]] std::string_view model_id() const noexcept override;
    [[nodiscard]] ReceptorLigandResponse
    evaluate(const ReceptorLigandRequest& request) const override;

  private:
    ReceptorLigandModelConfig config_;
};

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_ANALYTICAL_RECEPTOR_LIGAND_MODEL_HPP
