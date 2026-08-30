<!--
SPDX-FileCopyrightText: 2026 MEHLISSA contributors
SPDX-License-Identifier: CC-BY-4.0
-->

# Conservative Terminal Entity Ownership

## 1. Decision and scope

M4.12 turns the M4.6 retention, adhesion, and extravasation likelihoods into
optional sampled state changes without allowing an entity to disappear or have
two owners. The implementation adds:

- entity-disposition contract `1.0.0` in the model-coupling layer;
- a strict state-changing capillary profile;
- deterministic categorical sampling from the normalized M4.6 probabilities;
- a pending, retryable terminal hand-off in the organ-capillary coupler; and
- a `TerminalEntityStore` that becomes the explicit new owner.

The executable profile is:

```text
examples/capillary-models/synthetic-nanodevice-disposition-v1.json
data/schemas/capillary-entity-disposition-profile/1.0.0.schema.json
```

State changing is opt-in. A capillary bed without this profile retains the
non-state-changing M4.6 behavior and returns every entity.

## 2. Ownership lifecycle

One entity follows exactly one of two closing paths:

```text
organ -> open coupler ledger -> capillary
                              |-> pass-through -> organ acknowledges -> ledger closed
                              `-> terminal transfer -> target store acknowledges -> ledger closed
```

The capillary owns an entity while it is resident. On pass-through it emits the
existing entity-return transfer. On a sampled terminal result it emits an
`EntityDispositionTransfer` containing:

- entity ID and type;
- one of `retained`, `adhered`, or `extravasated`;
- profile, source model, and distinct terminal source port;
- target owner model and target compartment;
- decision time; and
- the deterministic draw and selected outcome probability.

Taking an outbound transfer moves ownership into the coupler's pending queue.
The original outstanding ID remains open until the organ or terminal store has
accepted it. A rejected target leaves the disposition pending and retryable.
Only after successful acceptance does the coupler erase the outstanding ID and
increment exactly one completion counter.

The terminal store rejects a wrong model, unknown compartment, invalid
transfer, or duplicate entity ID. Its records are persistent rather than
drainable because they represent owned state, not optional observations.

## 3. Sampling semantics

M4.6 supplies normalized probabilities in this order:

```text
pass-through, retained, adhered, extravasated
```

M4.12 obtains one open-interval uniform draw from the named simulation-context
stream for each completed matched entity. Cumulative probability intervals map
the draw to exactly one result. An unmatched entity remains certain
pass-through and consumes no draw.

The profile is bound to both the capillary model ID and the M4.6 observation
profile ID. Its three terminal outcomes must name one target owner model and
three distinct compartments. This prevents a state-changing profile from
silently using different rates or ambiguous ownership labels.

Sampling happens in stable resident order. A fixed master seed and named stream
therefore reproduce the same entity-to-outcome mapping across compatible host
step sizes. The transfer stores the draw so that every decision is auditable.

## 4. Executable result

The checked-in software case emits 256 `nanodevice` entities with master seed
`20260830`. Their M4.6 capillary residence is 0.6 s and the synthetic competing
rates remain `0.1`, `0.2`, and `0.3 s^-1`.

| Closing owner/outcome | Count |
|---|---:|
| returned to organ | `184` |
| tissue store: retained | `17` |
| tissue store: adhered | `25` |
| tissue store: extravasated | `30` |
| total | `256` |

Exactly 256 draws are consumed. The complete transfers, terminal counts, and
random-stream state are identical for 100 ms, 250 ms, and 500 ms host steps.
The test does not require sampled counts to equal expected probabilities; it
freezes a deterministic software reference and verifies ownership conservation.

A separate retry test deliberately supplies the wrong target store. No entity
is accepted there, every affected ID remains outstanding and pending, and the
same transfers subsequently close against the correct store.

## 5. Scientific boundary

The disposition frequencies have no physiological interpretation. They are
generated from the synthetic M4.6 rates and exist to verify sampling and
ownership. The compartments are abstract labels, not spatial anatomy.

Retention, adhesion, and extravasation are terminal in M4.12. The model does
not yet support detachment, remobilization, tissue movement, cellular uptake,
device degradation, reversible binding, feedback, or return to blood. The
terminal store establishes a safe hand-off boundary for M5; it is not itself a
tissue or cell simulator.

See [ADR-0032](../architecture/adr/0032-conservative-terminal-entity-ownership.md).
