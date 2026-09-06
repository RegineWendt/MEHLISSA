// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#ifndef MEHLISSA_MODELS_CELL_QUALIFIED_CD95_APOPTOSIS_MODEL_HPP
#define MEHLISSA_MODELS_CELL_QUALIFIED_CD95_APOPTOSIS_MODEL_HPP

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace mehlissa::models::cell {

inline constexpr std::string_view qualified_cd95_apoptosis_kind =
    "kallenberger_2014_minimal_cd95_caspase8_rk4";
inline constexpr std::string_view unresolved_model_native_unit = "unresolved-model-native";

enum class KallenbergerCase { cd95_hela, wild_type_hela };

enum class KallenbergerSpecies : std::size_t {
    cd95,
    fadd,
    disc,
    p55free,
    discp55,
    p30,
    p43,
    p18,
    p18inactive,
    bid,
    tbid,
    prnes_mcherry,
    prnes,
    mcherry,
    prer_mgfp,
    prer,
    mgfp,
    cd95l,
    count
};

inline constexpr std::size_t kallenberger_species_count =
    static_cast<std::size_t>(KallenbergerSpecies::count);

struct ModelNativeTime final {
    double value{};
};

struct ModelNativeStateValue final {
    double value{};
};

struct KallenbergerState final {
    std::array<double, kallenberger_species_count> values{};

    [[nodiscard]] ModelNativeStateValue at(KallenbergerSpecies species) const noexcept;
};

struct KallenbergerMinimalParameters final {
    double kon_fadd{};
    double koff_fadd{};
    double kdisc{};
    double kd216{};
    double kd374trans_p55{};
    double kd374trans_p43{};
    double kdiss_p18{};
    double kbid{};
    double kd374probe{};
    double kdr{};
    double kdl{};
};

struct KallenbergerSourceIdentity final {
    std::string_view accession;
    std::string_view role;
    std::string_view source_commit;
    std::string_view sbml_sha256;
    std::string_view licence;
};

struct KallenbergerMinimalDefinition final {
    KallenbergerSourceIdentity source;
    KallenbergerMinimalParameters parameters;
    KallenbergerState initial_state;
};

struct QualifiedCd95Stimulus final {
    std::string species_id;
    ModelNativeStateValue initial_value{};
    std::string unit_semantics;
};

struct QualifiedCd95Request final {
    std::string request_id;
    KallenbergerCase source_case{KallenbergerCase::cd95_hela};
    QualifiedCd95Stimulus stimulus;
    ModelNativeTime end_time{};
    ModelNativeTime output_interval{};
    ModelNativeTime maximum_internal_step{};
};

struct KallenbergerObservables final {
    ModelNativeStateValue prer_mgfp{};
    ModelNativeStateValue prnes_mcherry{};
    ModelNativeStateValue p43{};
    ModelNativeStateValue p18{};
};

struct QualifiedCd95Sample final {
    ModelNativeTime model_time{};
    KallenbergerState state;
    KallenbergerObservables observables;
};

struct QualifiedCd95Response final {
    std::string request_id;
    std::string_view model_kind;
    KallenbergerSourceIdentity source;
    std::string_view time_unit;
    std::string_view state_unit;
    std::size_t integration_steps{};
    std::vector<QualifiedCd95Sample> samples;
};

[[nodiscard]] std::string_view species_id(KallenbergerSpecies species) noexcept;
[[nodiscard]] const KallenbergerMinimalDefinition&
kallenberger_minimal_definition(KallenbergerCase source_case);

// This class is the source-equation layer. It contains only the 13 reactions and
// assignment rule represented by the selected CC0 SBML source family.
class KallenbergerMinimalMechanism final {
  public:
    explicit KallenbergerMinimalMechanism(KallenbergerMinimalParameters parameters);

    [[nodiscard]] double assigned_cd95_activity(const KallenbergerState& state) const;
    [[nodiscard]] KallenbergerState derivative(const KallenbergerState& state) const;
    [[nodiscard]] std::vector<QualifiedCd95Sample> integrate(const KallenbergerState& initial_state,
                                                             ModelNativeTime end_time,
                                                             ModelNativeTime output_interval,
                                                             ModelNativeTime maximum_internal_step,
                                                             std::size_t& integration_steps) const;

  private:
    KallenbergerMinimalParameters parameters_;
};

// This class is the typed M5 boundary. It locks source identity, stimulus and
// unresolved unit semantics while delegating equations to the mechanism above.
class QualifiedCd95ApoptosisAdapter final {
  public:
    [[nodiscard]] std::string_view kind() const noexcept;
    [[nodiscard]] QualifiedCd95Response evaluate(const QualifiedCd95Request& request) const;
};

} // namespace mehlissa::models::cell

#endif // MEHLISSA_MODELS_CELL_QUALIFIED_CD95_APOPTOSIS_MODEL_HPP
