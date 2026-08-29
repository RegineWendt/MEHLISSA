<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Independent Pulmonary Lobar Perfusion Validation (M3.17)

## Result

M3.17 evaluates the executable five-bed v7 pulmonary model against a source
that was not used to calibrate it. Bourhis et al. report mean relative
perfusion for all five lobes from 73 supine V/Q SPECT/CT examinations judged
strictly normal. Both published reconstruction series pass the declared
engineering compatibility criteria without fitting any v7 parameter.

| V/Q SPECT reconstruction | Five-lobe RMSE | Maximum lobe error | Right-lung error | Result |
|---|---:|---:|---:|---|
| without attenuation correction | 0.990 percentage points | 1.877 percentage points | 1.047 percentage points | pass |
| with attenuation correction | 1.033 percentage points | 1.923 percentage points | 1.192 percentage points | pass |

This closes the immediate M3.16 question of whether the fixed v7 lobe shares
are compatible with an independent regional measurement. It does not validate
dynamic redistribution, posture changes, disease, individual anatomy, or
clinical use.

## Evidence separation

The v7 lobe fractions were derived from the Lee et al. dual-energy CT PBV
cohort. M3.17 uses Bourhis et al., a different author group, cohort, and
modality. The evaluator compares validation-source URLs with every model-card
source URL and fails if one is reused.

The validation file transcribes only aggregate values from Table 1 of the
openly licensed article. It does not redistribute the article or participant
records. The source describes 73 consecutive clinically referred examinations
with normal V/Q SPECT/CT, no low-dose CT parenchymal or pleural abnormality,
and excluded pulmonary-disease history. Two nuclear physicians confirmed that
the examinations were strictly normal.

## Published observations and normalization

The source reports lobe contributions as percentages rounded for publication.
The attenuation-corrected series sums to 100.0%, while the uncorrected series
sums to 100.1%. MEHLISSA stores those values unchanged. The schema requires the
explicit policy
`renormalize_reported_rounded_fractions_to_unit_sum`; the evaluator divides
each reported value by its series total before comparison. It neither silently
changes the source table nor compares a 100.1% partition to a unit-sum model.

| Lobe | v7 predicted | SPECT without AC, normalized | SPECT with AC |
|---|---:|---:|---:|
| right upper | 22.255% | 22.178% | 21.600% |
| right middle | 8.783% | 9.690% | 8.000% |
| right lower | 25.653% | 23.776% | 25.900% |
| left upper | 23.631% | 24.176% | 22.900% |
| left lower | 19.677% | 20.180% | 21.600% |

## Acceptance protocol

Both reconstruction series must meet all three conditions:

1. every absolute per-lobe error is at most 3 percentage points;
2. five-lobe RMSE is at most 2 percentage points; and
3. the absolute right-lung aggregate error is at most 3 percentage points.

The thresholds were declared in the versioned case before its first executable
evaluation and reflect the source's reconstruction sensitivity. The v7 values
were already known, so this is a transparent engineering compatibility check,
not a blinded preregistered statistical-equivalence study. The two source
series are alternate reconstructions of the same examinations and must not be
counted as independent cohorts.

## Reproducible implementation

- validation case:
  `data/validation/pulmonary-lobar-perfusion/healthy-normal-spect-v1.json`;
- strict schema:
  `data/schemas/pulmonary-lobar-perfusion-validation/1.0.0.schema.json`;
- loader and evaluator:
  `models/organ/src/pulmonary_lobar_perfusion_validation.cpp`;
- executable tests:
  `tests/pulmonary_lobar_perfusion_validation_tests.cpp`.

The evaluator constructs the v7 model through the normal factory and reads its
actual `PulmonaryParallelBedsState`. It rejects missing or duplicate beds,
invalid partitions, the wrong definition, a non-parallel implementation, and
calibration-source reuse. It reports normalized reference values, predicted
fractions, signed residuals, maximum error, RMSE, and right-lung error.

Run the dedicated checks after a Debug build:

```powershell
ctest --test-dir build/windows-msvc -C Debug `
  -R "SPECT|Lobar validation rejects" --output-on-failure
```

## Interpretation and remaining limits

The result supports v7 as a reproducible healthy-adult, supine, mean regional
perfusion reference. It is stronger than aggregate equivalence alone because
all five model outputs are compared to calibration-disjoint measurements.

It remains limited because the source cohort consists of clinically referred
patients selected for normal findings rather than population-sampled healthy
volunteers; only aggregate means are available in the repository; SPECT count
fractions and DE-CT PBV proxies are related but not identical physical
quantities; and no response to gravity, exercise, hypoxia, embolism, or other
disease is evaluated. A future dynamic regional model needs separate data and
acceptance rules for those states.

## Source

Bourhis D, Robin P, Essalah A, Abgral R. V/Q SPECT for the Assessment of
Regional Lung Function: Generation of Normal Mean and Standard Deviation 3-D
Maps. *Front Med*. 2020;7:143.
<https://doi.org/10.3389/fmed.2020.00143>

The article is distributed under CC BY 4.0.
