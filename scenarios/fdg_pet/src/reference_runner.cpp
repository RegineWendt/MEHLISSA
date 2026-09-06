// SPDX-FileCopyrightText: 2026 MEHLISSA contributors
// SPDX-License-Identifier: MPL-2.0

#include "mehlissa/scenarios/fdg_pet/fdg_pet_model.hpp"
#include <iomanip>
#include <iostream>
#include <vector>

int main() {
    namespace fdg = mehlissa::scenarios::fdg_pet;
    namespace core = mehlissa::core;
    const std::vector<fdg::Frame> frames{
        {core::seconds(0), core::seconds(10)},  {core::seconds(10), core::seconds(20)},
        {core::seconds(30), core::seconds(30)}, {core::minutes(1), core::minutes(4)},
        {core::minutes(5), core::minutes(5)},   {core::minutes(10), core::minutes(10)},
        {core::minutes(20), core::minutes(20)}, {core::minutes(40), core::minutes(30)}};
    const auto output =
        fdg::simulate(fdg::source_disjoint_reference_candidate(), frames, core::seconds(0.5));
    std::cout << "frame_start_s,frame_duration_s,aortic_bq_ml,lung_bq_ml,liver_bq_ml,kidney_bq_ml,"
                 "bladder_bq\n"
              << std::setprecision(12);
    for (const auto& row : output) {
        std::cout << core::in_seconds(row.frame.start) << ','
                  << core::in_seconds(row.frame.duration) << ','
                  << fdg::in_becquerels_per_milliliter(row.aortic_input) << ','
                  << fdg::in_becquerels_per_milliliter(row.lung) << ','
                  << fdg::in_becquerels_per_milliliter(row.liver) << ','
                  << fdg::in_becquerels_per_milliliter(row.kidney) << ','
                  << fdg::in_becquerels(row.urinary_bladder) << '\n';
    }
}
