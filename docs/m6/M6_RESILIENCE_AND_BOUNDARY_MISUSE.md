<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# M6.7 Nano-IoT Resilience and Boundary-Misuse Scenarios

## Purpose

M6.7 turns the failure, miscontrol, trace-integrity, capacity, and resource
cases accumulated across M6 into one strict executable catalog. The catalog
asks a narrow software question: when a prescribed communication failure or
invalid boundary input occurs, is it reported at the intended boundary without
an unintended downstream state change?

This is a resilience and contract-governance experiment, not a cybersecurity
claim. It does not implement authentication, encryption, signatures, key
management, intrusion detection, clinical authorization, or fail-safe medical
control.

## Versioned inputs

```text
examples/iot-models/synthetic-iot-resilience-scenarios-v1.json
data/schemas/iot-resilience-scenario-profile/1.0.0.schema.json
```

The typed `ResilienceScenarioProfile` loader requires exactly one declaration
for every mandatory injection, unique scenario and source identities, a
consistent injection-to-disposition mapping, nonempty protected properties,
explicit excluded claims, and `protected_state_unchanged: true` for every case.
The baseline is `synthetic-ban-station-v1`.

## Executable scenario matrix

| Injection | Category | Checked boundary | Expected disposition | Protected observation |
|---|---|---|---|---|
| uplink loss | availability | BAN transport | `lost` | station receives no measurement; attempt and bytes remain counted |
| uplink corruption | integrity | BAN transport | `corrupted` | station receives no measurement; corruption and energy remain counted |
| frame expiry | availability | BAN transport | `expired` | nominal delivery cannot bypass the absolute validity deadline |
| unauthorized target | miscontrol | station governance | `denied_target` | no command is approved |
| disallowed content type | miscontrol | station governance | `denied_content_type` | no command is approved |
| correlation mismatch | traceability | station governance | `denied_correlation_mismatch` | unrelated experiments cannot be joined |
| duplicate request | traceability | station governance | `denied_duplicate_request` | approval count does not increase twice |
| approved-command capacity | capacity | station governance | `denied_capacity` | the configured command bound is not exceeded |
| command replay | traceability | gateway adapter | rejected invariant | the gateway accepts one decision identity only once |
| external-response identity mismatch | integrity | external-simulator adapter | rejected invariant | no BAN result is accepted from mismatched simulator output |
| external-attempt capacity | capacity | external-simulator adapter | rejected invariant | the client is not called beyond the configured bound |
| local transmission exhaustion | resource | nanodevice | rejected resource | count and remaining energy do not change after rejection |

The executable checks are in `tests/iot_resilience_scenario_tests.cpp`. They use
the production M6.1, M6.5, and M6.6 components rather than a separate mock
policy engine. Existing M6 tests continue to cover local routing, relay loss,
gateway capacity, malformed contracts, and the successful end-to-end path.

## Result interpretation

Prescribed non-delivery is a modeled result, not a software exception. It is
therefore included in attempted-message, attempted-byte, outcome, and energy
metrics. Inputs that violate a contract, trace, allow-list, replay rule,
capacity, or resource invariant are rejected before the protected downstream
state changes.

This distinction keeps communication evidence separate from biological
results. A lost frame is still a communication attempt; an unauthorized or
replayed command is not a physiological outcome. A successful control-message
delivery remains only a communication event and does not execute drug release.

## Security scope and limitations

The synthetic threat model covers malformed, replayed, misrouted, disallowed,
or excess input at explicit MEHLISSA boundaries and deterministic transport
non-delivery. It protects causal/correlation identity, configured target and
content policy, replay uniqueness, bounded capacities/resources, downstream
state on rejection, and separate communication accounting.

It does not support claims about:

- authenticated users, devices, stations, or simulators;
- confidentiality, cryptographic integrity, secure boot, or key lifecycle;
- resistance to an adaptive attacker, denial of service, or traffic analysis;
- real network failure probabilities, noise, interference, or availability;
- clinical decision quality, treatment authorization, or medical-device
  safety; or
- biological safety after a communication event.

Any later security implementation must add a new threat model, versioned
contracts, key/identity ownership, adversary assumptions, and independent
verification. It must not reinterpret the M6.7 allow-list as authentication.
