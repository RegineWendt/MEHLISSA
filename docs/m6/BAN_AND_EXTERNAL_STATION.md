<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# BAN and External Analysis/Control Station

## Purpose and boundary

M6.5 closes the first traceable software path from a local gateway measurement
to an external analysis/control station and back to a local actuator:

```text
local measurement -> ActiveGateway -> GatewayBanAdapter
  -> BanTransportAdapter -> ExternalAnalysisControlStation
  -> explicit transport-policy decision -> BanTransportAdapter
  -> GatewayBanAdapter -> ActiveGateway -> bounded local route -> actuator
```

This is a software-contract reference, not a wearable-network implementation.
The BAN transports typed frames and reports communication metrics without
introducing a dependency from physiology, cell models, or local nanodevices to
a particular radio, operating system, or network simulator.

## Versioned contracts

`BanFrame` version `1.0.0` carries either a `GatewayMeasurement` uplink or a
`GovernedGatewayCommand` downlink. It preserves endpoint, experiment
correlation, source-event, time, validity, size, and typed payload identity.
Payload kind and redundant frame metadata must agree; mismatches are rejected.

`GatewayBanAdapter` has two responsibilities:

- wrap a measurement published by its configured gateway and remember its
  measurement/correlation identity; and
- accept a returned command only from the configured station, for the
  configured gateway, within validity and capacity, with a unique decision ID,
  and causally linked to a measurement previously published by this adapter.

`ExternalAnalysisControlStation` accepts measurements only from configured
gateways. Its explicit command policy requires:

- a unique request identity;
- a measurement already received by this station;
- command creation no earlier than station receipt of that measurement;
- exact experiment-correlation agreement;
- an allow-listed target device; and
- an allow-listed command content type plus remaining approval capacity.

Policy denials are normal typed outcomes rather than simulated software
failures. Only an approved decision can be converted into a downlink frame.
The decision identity, station, gateway, source measurement, and original
`GatewayCommand` remain visible throughout the return path.

## Replaceable BAN transport

`BanTransportAdapter` is the technology-neutral extension point:

```cpp
class BanTransportAdapter {
  public:
    virtual ~BanTransportAdapter() = default;
    virtual std::string_view kind() const noexcept = 0;
    virtual std::string_view adapter_id() const noexcept = 0;
    virtual BanTransferResult transfer(const BanFrame& frame) = 0;
};
```

The M6.5 `ScheduledBanTransportAdapter` is deterministic. It applies configured
latency, transmitter/receiver/link energy, and a repeating delivered/lost/
corrupted outcome sequence. Validity expiry takes precedence. A
`BanCommunicationSession` validates adapter results and records counts, bytes,
loss, corruption, expiry, delivered latency, and the three energy categories
using the shared communication-metrics contract.

An external network simulator can implement this interface in M6.6 without
changing the gateway, station, local cluster, or biological models.

## Reference profile and verification

The strict profile is:

```text
examples/iot-models/synthetic-ban-station-v1.json
data/schemas/ban-station-profile/1.0.0.schema.json
```

The positive reference publishes a 256-byte lung-detection measurement over a
synthetic 10 ms gateway-to-station link. The station approves a 128-byte
activation command linked to that measurement, returns it over a 15 ms link,
and the active gateway sends it through the existing two-hop local route to the
actuator in 20 ms. Exact BAN transmitter, receiver, and link energy is reported
separately from the local-route energy.

Verification also covers unknown source measurements, acausal time order,
correlation mismatch, non-allow-listed targets, duplicate requests, command
replay, prescribed BAN loss/corruption, and validity expiry. These cases establish deterministic
contract and accounting behavior, not real-network performance.

## Scientific and safety limits

All M6.5 capacities, delays, energy values, and outcomes are synthetic software
test values. The scheduled adapter is not Bluetooth, IEEE 802.15.6, cellular,
Internet, tissue propagation, or validated hardware. It has no queue,
contention, throughput limit, mobility, retry, protocol handshake, or measured
error distribution.

“Governed” means only that the configured simulation transport policy makes an
explicit, traceable allow/deny decision and that the gateway checks the causal
return path. It does not mean human identity authentication, cryptographic
integrity, authorization by a clinician, treatment planning, dose safety,
medical-device risk control, or fail-safe operation. Receipt at the actuator
is still communication only: it does not release payload or alter physiology.

See [ADR-0046](../architecture/adr/0046-ban-adapter-and-governed-station-loop.md)
for the architectural decision.
