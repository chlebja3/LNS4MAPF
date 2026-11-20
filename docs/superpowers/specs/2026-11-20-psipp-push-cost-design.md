# PSIPP: push cost and per-push-count closed list

## Intent

PSIPP (SIPP + Push and Swap) is the extra contribution of the paper on top of the
thesis results (SIPP heuristics, blocked destroy operator, suboptimal repair).
Its job is to find *any* feasible initial solution for LNS, faster and on denser
instances than plain SIPP-based prioritized planning. Solution cost of the initial
solution is secondary; success rate and time to first solution are what matter.

The baseline sweep (CIIRC, 60 s limit) shows PSIPP helps near the capacity limit
(e.g. warehouse 550: 25 % → 76–82 % success; random-32-32-20 225: 29.6 s → 1.9 s),
gives nothing on large maps, and everything fails past the limit
(random-32-32-20 ≥ 275, room-64-64-8 ≥ 400, empty-32-32 500).

## Goals

- Make the PSIPP single-agent search more general with two new options whose
  defaults reproduce today's behaviour exactly.
- Measure, not guess, whether they help: success rate and time to first solution
  vs. plain SIPP, plus diagnostics on *why* attempts fail.

## Non-goals

- Changing what happens after a push (cycle handling, priority swaps, local repair).
  Decided later, based on the failure-reason counters added here.
- Using PSIPP in the LNS repair step.
- GUI changes.

## Anti-goals

- Changing default behaviour. With `push_cost = inf` and the shared closed list,
  results must match the current code (same comparator code path).

## Approaches considered

1. **Push cost + per-push-count closed list in the single-agent search** (picked).
   Cheap, general (inf recovers current behaviour), directly testable.
2. Smarter handling of pushed agents during PP (priority swap on cycles, repeated
   pushes, local repair). Possibly the bigger win on dense instances, but it is
   unclear how often cycles are the failure cause; postponed until the counters
   from this work say so.
3. PSIPP inside LNS repair. Rejected for now: the initial-solution phase must be
   convincing first, and repair already starts from a feasible solution.

## Design

### PP loop: which planner plans which agent (new `LNS_settings` field `sipp_first`, bool, default `true`)

- `true` (variant C, applied 2026-11-20): the newly added agent tries plain SIPP
  first and falls back to PSIPP; agents pushed out of the way are replanned
  directly by PSIPP. This replaced a bug where the SIPP call planned the newly
  added agent instead of the pushed one (its path could be stored for the wrong
  agent).
- `false`: every agent, including the newly added one, is planned directly by
  PSIPP (the behaviour of commit 875672a).
- Invariant, asserted: every path stored for agent `a` starts at `a`'s start and
  ends at `a`'s goal.

### `push_cost` (new `PSIPP_settings` field, double, default `inf`)

- `inf`: existing lexicographic comparator (pushes first, then f, h, h2, h3, random)
  is used unchanged.
- finite `c`: nodes are ordered by `f + c * pushes`, then the existing tie-breaks
  (h, h2, h3, random). `max_pushes` still caps the number of pushes.
- Applies to `PSIPP_mine` and `PSIPP_mine_ap` (they share the comparator class);
  the suboptimal implementations keep the lexicographic comparator.

### `closed_list_per_push_count` (new `PSIPP_settings` field, bool, default `false`)

- `false`: the shared per-cell `known_min` / `known_max` span (today).
- `true`: separate `known_min` / `known_max` arrays per push count
  0..`max_pushes`; a node with p pushes is only pruned by spans recorded by nodes
  with p pushes. Memory and state space grow at most `(max_pushes + 1)×`.
  Per-push-*set* keying was rejected because the number of sets explodes.

### Diagnostics (LNS log / benchmark output)

Per PP attempt, count:
- failure because PSIPP found no path for an agent;
- failure because of a detected cycle (an agent pushed twice);
- number of agents replanned because they were pushed.

### CLI wiring (`main.cpp`)

Wire `--psipp_implementation` and `--maxPushes` (parsed today but ignored), and
add `--pushCost`, `--closedListPerPushCount` and `--sippFirst`.

## Semantics notes

- `max_pushes` bounds the number of *distinct* pushed agents. The `pushed_new` branches inside the
  `for (auto& additional_const : current->pushed)` loop of `PSIPP.cpp` (`plan_psipp_mine_ap`, roughly lines 796-829,
  and the same in the other implementations) add a push constraint without a cap, so the number of push
  *constraints* on a node can exceed `max_pushes` (pre-existing behaviour). `push_cost` charges per push constraint,
  and the per-count closed list puts every count >= `max_pushes` in the last layer.
- With `sipp_first = true`, if a pushed agent pushes the newly added agent back, that agent re-enters `to_replan` and
  tries SIPP first again (`LNS.cpp` ~309). This is harmless (the path is valid and correctly owned) but a minor
  deviation from "pushed agents go directly to PSIPP".
- With a finite `push_cost` and the shared closed list, a pushed node with lower f can be popped before a push-free
  node at the same cell and prune it, so push-free solutions may be lost. The per-count closed list removes this;
  keep it in mind when comparing `cl0` vs `cl1`.

## Experiment

- Grid: `sipp_first` ∈ {true, false} × `push_cost` ∈ {inf, 0, 2, 10, 50} ×
  closed list {shared, per count}, `max_pushes = 5`, `PSIPP_mine_ap`, plus plain
  SIPP (`SIPP_mine_ap`) as reference.
- Maps and agent counts: the 7 maps of the baseline sweep, 25 random scenarios
  each, 60 s limit, CIIRC `compute` partition.
- Metrics: success rate, time to first solution, PP attempts, diagnostics above.

## Left to the builder

- Exact data structure for the per-count closed list.
- Avoiding the per-node copy of the pushed vector (parent-linked list) is an
  allowed optimization if it is behaviour-preserving.
