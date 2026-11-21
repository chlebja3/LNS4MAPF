# PSIPP push cost, per-push-count closed list, SIPP-first switch — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make PSIPP's search configurable (push cost, per-push-count closed list, SIPP-first switch), add failure diagnostics, and run the experiment grid on CIIRC that decides whether these ideas beat plain SIPP.

**Architecture:** All options live in the existing settings classes (`PSIPP_settings`, `LNS_settings`) with defaults that reproduce current behaviour. The PP loop in `LNS::PrioritizedPlanning` gets the switch and the diagnostics; the PSIPP comparator and closed list get the two search options. A headless driver plus cluster scripts run the grid.

**Tech Stack:** C++17, CMake, GoogleTest (`tests/`), Boost.Heap, Slurm on CIIRC (`compute` partition only), Python 3 for aggregation.

**Spec:** `docs/superpowers/specs/2026-11-20-psipp-push-cost-design.md`

## Global Constraints

- Defaults must reproduce current behaviour: `push_cost = inf` uses the existing lexicographic comparator code path unchanged; `closed_list_per_push_count = false` uses the existing shared `known_min`/`known_max`; `sipp_first = true` is variant C (already in `src/LNS.cpp`).
- Grid values (verbatim): `sipp_first` ∈ {true, false}; `push_cost` ∈ {inf, 0, 2, 10, 50}; closed list {shared, per count}; `max_pushes = 5`; `PSIPP_mine_ap`; reference `SIPP_mine_ap`.
- Experiment: 7 maps of the baseline sweep, 25 random scenarios, 60 s limit, CIIRC `compute` partition. Never use the `gpu` partition, never use Clara.
- Python: type hints only in function headers; all imports at top of file; no extra exceptions (fail fast).
- C++: all includes at the top; use `assertm` for invariants.
- Git: work on branch `push-and-sipp`; `git add` only the files named in the task (the working tree has unrelated user changes); no `Co-Authored-By` line in commit messages.
- Build/test: `cd my_code/my_solver/build && make -j8 && ctest` (tests need the in-repo `build/` dir because of relative test-map paths). Asserts are off in this build (`ENABLE_ASSERTS=OFF`); for assert-based checks configure a second dir `build_assert` with `cmake .. -DENABLE_ASSERTS=ON`.

## Review Focus

- `push_cost = 0` with `max_pushes = 5`: search may prefer pushes everywhere; PP must still return only valid solutions or fail cleanly (no crash, no infinite push loop) — covered by Task 3's PP validity test with `push_cost = 0`.
- Per-count closed list must index by the **neighbor's** push count (after deciding whether this expansion pushes), not the parent's — covered by Task 4's `MustPush` test with the flag on (the push node would be pruned by the 0-push span otherwise).
- `sipp_first = false` on an instance where no pushes are needed must give a valid solution — covered by Task 2's den520d test.
- A failed PP attempt must leave no path stored under the wrong agent — covered by Task 1's per-path start/goal invariant, checked on failed attempts too.
- Timeouts in the cluster driver produce truncated output lines; the aggregator must count them as failures, not crash — covered by Task 7's aggregator test line.

---

### Task 1: Path-owner invariant and regression test for the wrong-agent SIPP bug

**Files:**
- Modify: `my_code/my_solver/src/LNS.cpp` (PP loop, right after a non-empty `tp_path` is obtained, before `add_trajectory`)
- Create: `my_code/my_solver/tests/test_maps/random-32-32-20.map` (copy of `MAPF-benchmark/mapf-map/random-32-32-20.map`)
- Create: `my_code/my_solver/tests/test_scen/random-32-32-20-random-5.scen` (copy of `MAPF-benchmark/mapf-scen-random/scen-random/random-32-32-20-random-5.scen`)
- Test: `my_code/my_solver/tests/src/psipp_tests.cpp`

**Interfaces:**
- Produces: helper in the test file `auto paths_owned_correctly(const Solution& sol, const Instance& instance) -> bool` — true iff every non-empty `sol.paths[a]` has `front().location == instance.get_start_locations()[a]` and `back().location == instance.get_goal_locations()[a]`.

- [ ] **Step 1: Write the failing test** `TEST(PSIPPTest, PushedAgentKeepsOwnPath)`: random-32-32-20, 225 agents, `std::mt19937 rng(5)`, `LNS_settings(0, 30, {DESTROY_TYPE::RANDOM, 10}, {SIPP_implementation::SIPP_mine_ap, INFO_type::no_info, 1.0}, {PSIPP_implementation::PSIPP_mine_ap, INFO_type::no_info, 1.0, 1, 5}, true, true)`. Loop 500 attempts: `Solution sol = lns.PrioritizedPlanning(); ASSERT_TRUE(paths_owned_correctly(sol, *instance)) << "attempt " << i;` then `lns.get_constraint_manager().reset(); lns.already_planned.clear();`. Stop early when `sol.feasible` (also `EXPECT_TRUE(sol.is_valid(*instance))` then).
- [ ] **Step 2: Verify it fails on the buggy line.** Temporarily change the SIPP call in the PP loop back to `planner->plan(agent_id, already_planned)` without the `*it == agent_id` guard, build, run `./tests/unit_tests --gtest_filter=PSIPPTest.PushedAgentKeepsOwnPath`. Expected: FAIL. If it passes, raise the attempt count until it fails (variant A triggered within 420 attempts on seed 5) but keep runtime under 60 s; if it still cannot be made to fail, report that instead of weakening the test. Restore the variant C line.
- [ ] **Step 3: Add the invariant** in `src/LNS.cpp`: `assertm(tp_path.front().location == instance.get_start_locations()[*it] && tp_path.back().location == instance.get_goal_locations()[*it], "Path stored for the wrong agent.");`
- [ ] **Step 4: Run** `make -j8 && ctest` in `build/` and the single test in `build_assert/`. Expected: all pass.
- [ ] **Step 5: Commit** `git add src/LNS.cpp tests/src/psipp_tests.cpp tests/test_maps/random-32-32-20.map tests/test_scen/random-32-32-20-random-5.scen && git commit -m "Fix PP replanning pushed agents with the wrong agent's SIPP; add path-owner invariant and test"`

### Task 2: `sipp_first` switch

**Files:**
- Modify: `my_code/my_solver/inc/LNS.h` (`LNS_settings`)
- Modify: `my_code/my_solver/src/LNS.cpp` (PP loop guard)
- Test: `my_code/my_solver/tests/src/psipp_tests.cpp`

**Interfaces:**
- Produces: `LNS_settings::sipp_first` (bool), constructor gains trailing parameter `bool sipp_first_ = true` after `use_psipp_`.

- [ ] **Step 1: Write the failing test** `TEST(PSIPPTest, SippFirstSwitch)`: for `sipp_first` in {true, false}: den520d 100 agents (`den520d-random-0.scen`) and must_push 2 agents, `PSIPP_mine_ap`, `use_psipp = true`, seed 0; `Solution sol = lns.PrioritizedPlanning(); EXPECT_TRUE(sol.is_valid(*instance)); EXPECT_TRUE(paths_owned_correctly(sol, *instance));`
- [ ] **Step 2: Run** — expected: compile error (no such constructor argument).
- [ ] **Step 3: Implement**: guard becomes `if (settings.sipp_first && *it == agent_id)`.
- [ ] **Step 4: Run** `make -j8 && ctest`. Expected: all pass.
- [ ] **Step 5: Commit** `git add inc/LNS.h src/LNS.cpp tests/src/psipp_tests.cpp && git commit -m "Add sipp_first switch to LNS settings"`

### Task 3: `push_cost` in the PSIPP comparator

**Files:**
- Modify: `my_code/my_solver/inc/PSIPP.h` (`PSIPP_settings`, `PSIPPNodeComparator`)
- Modify: `my_code/my_solver/src/PSIPP.cpp:44-72` (comparator), `:199`, `:514` (open-list construction)
- Test: `my_code/my_solver/tests/src/psipp_tests.cpp`

**Interfaces:**
- Produces: `PSIPP_settings::push_cost` (double), constructor gains `double push_cost_ = std::numeric_limits<double>::infinity()` after `max_pushes_`; `PSIPPNodeComparator(std::mt19937* rnd_generator_, double push_cost_ = std::numeric_limits<double>::infinity())`.

- [ ] **Step 1: Write the failing tests**
  - `TEST(PSIPPComparator, InfinitePushCostIsLexicographic)`: node `a` with 0 pushes, f = 100; node `b` with 1 push, f = 10 (build via the `PSIPPNode(location, interval, g, h, h2, h3, parent, pushed)` constructor; one dummy `Constraint` in `b.pushed`). `PSIPPNodeComparator cmp(&rng);` → `EXPECT_FALSE(cmp(&a, &b)); EXPECT_TRUE(cmp(&b, &a));`
  - `TEST(PSIPPComparator, FinitePushCost)`: same nodes, `PSIPPNodeComparator cmp(&rng, 5.0);` → score a = 100, b = 15 → `EXPECT_TRUE(cmp(&a, &b)); EXPECT_FALSE(cmp(&b, &a));` and with `cmp(&rng, 200.0)` → `EXPECT_FALSE(cmp(&a, &b));`
  - `TEST(PSIPPTest, PushCostPPValid)`: for `push_cost` in {0, 2, 10, 50}: den520d 100 agents and must_push 2 agents, `PSIPP_mine_ap`, `max_pushes = 5`; `EXPECT_TRUE(sol.is_valid(*instance))`.
- [ ] **Step 2: Run** — expected: compile errors.
- [ ] **Step 3: Implement**: if `push_cost` is infinite, run the existing body unchanged; otherwise compare `f + push_cost * pushed.size()` first, then the existing tie-breaks h, h2, h3, random. Pass `settings.push_cost` when constructing the open list in `plan_psipp_mine` and `plan_psipp_mine_ap`; suboptimal comparator untouched.
- [ ] **Step 4: Run** `make -j8 && ctest`. Expected: all pass.
- [ ] **Step 5: Commit** `git add inc/PSIPP.h src/PSIPP.cpp tests/src/psipp_tests.cpp && git commit -m "Add push_cost option to PSIPP node ordering"`

### Task 4: Per-push-count closed list

**Files:**
- Modify: `my_code/my_solver/inc/PSIPP.h` (`PSIPP_settings`)
- Modify: `my_code/my_solver/src/PSIPP.cpp` (constructor `:34-35`, `plan_psipp_mine_ap` around `:541-597` and `:640-700`)
- Test: `my_code/my_solver/tests/src/psipp_tests.cpp`

**Interfaces:**
- Consumes: `PSIPP_settings` from Task 3.
- Produces: `PSIPP_settings::closed_list_per_push_count` (bool), constructor gains `bool closed_list_per_push_count_ = false` after `push_cost_`.

- [ ] **Step 1: Write the failing test** `TEST(PSIPPTest, PerPushCountClosedListValid)`: flag on, `max_pushes = 5`, `push_cost` in {inf, 2}; must_push 2 agents and den520d 100 agents; `EXPECT_TRUE(sol.is_valid(*instance))`.
- [ ] **Step 2: Run** — expected: compile error.
- [ ] **Step 3: Implement** in `plan_psipp_mine_ap` only: `known_min`/`known_max` sized `num_cells * (max_pushes + 1)`, index `location + num_cells * p` where `p = pushed.size()` when the flag is on, else `p = 0`. On pop, check and update with the popped node's `p`. For neighbors, the pruning check must use the neighbor's push count, so move it after `new_pushed` is computed (when the flag is off keep the current position so default behaviour is identical). The consistency `assertm` at `:646` uses the same index.
- [ ] **Step 4: Run** `make -j8 && ctest` and `build_assert` ctest. Expected: all pass.
- [ ] **Step 5: Commit** `git add inc/PSIPP.h src/PSIPP.cpp tests/src/psipp_tests.cpp && git commit -m "Add per-push-count closed list option to PSIPP"`

### Task 5: PP diagnostics counters

**Files:**
- Modify: `my_code/my_solver/inc/LNS.h` (`Logger`)
- Modify: `my_code/my_solver/src/LNS.cpp` (PP loop: the `tp_path.empty()` failure, the cycle failure, the push loop)
- Test: `my_code/my_solver/tests/src/psipp_tests.cpp`

**Interfaces:**
- Produces: `Logger::pp_fail_no_path`, `Logger::pp_fail_cycle`, `Logger::pp_pushed_replanned` (int, default 0, cumulative over the LNS object's lifetime).

- [ ] **Step 1: Write the failing test** `TEST(PSIPPTest, DiagnosticsCounters)`: must_push 2 agents, PSIPP, one `PrioritizedPlanning()` → `EXPECT_GE(lns.log.pp_pushed_replanned, 1); EXPECT_EQ(lns.log.pp_fail_no_path + lns.log.pp_fail_cycle, 0);`. Plus `biparted_5_5.map` with `empty_5_5_scen_1.scen`, 1 agent (same instance as `PSIPPTest.NoPathPossible`), `sipp_first = false` → `PrioritizedPlanning()` returns infeasible and `EXPECT_EQ(lns.log.pp_fail_no_path, 1); EXPECT_EQ(lns.log.pp_fail_cycle, 0);`.
- [ ] **Step 2: Run** — expected: compile error.
- [ ] **Step 3: Implement**: increment at the two failure returns and once per id inserted into `erased_from_to_replan`.
- [ ] **Step 4: Run** `make -j8 && ctest`. Expected: all pass.
- [ ] **Step 5: Commit** `git add inc/LNS.h src/LNS.cpp tests/src/psipp_tests.cpp && git commit -m "Count PP failure reasons and replanned pushed agents"`

### Task 6: CLI wiring in `main.cpp`

**Files:**
- Modify: `my_code/my_solver/src/main.cpp:40-45` (options), `:83-95` (reading), `:148` (settings)

- [ ] **Step 1: Implement**: parse `--psipp_implementation` with `magic_enum::enum_cast<PSIPP_implementation>` (same warning pattern as SIPP), use `--maxPushes`, add `--pushCost` (double, default inf), `--closedListPerPushCount` (bool, default false), `--sippFirst` (bool, default true), and pass them into `PSIPP_settings` / `LNS_settings`.
- [ ] **Step 2: Verify**: (binary name from `add_executable` for `src/main.cpp` in `CMakeLists.txt`) `<solver> -m ../tests/test_maps/must_push.map -a ../tests/test_scen/must_push.scen -k 2 -G 0 -t 2 -s 0 --use_psipp 1 --psipp_implementation PSIPP_mine_ap --maxPushes 5 --pushCost 2 --closedListPerPushCount 1 --sippFirst 0` exits 0; `--psipp_implementation bogus` prints the WARNING line.
- [ ] **Step 3: Commit** `git add src/main.cpp && git commit -m "Wire PSIPP options into the CLI"`

### Task 7: Headless benchmark driver and cluster scripts

**Files:**
- Create: `my_code/my_solver/src/experiments/psipp_init_bench.cpp` (from `scratchpad/bench/init_bench.cpp`)
- Modify: `my_code/my_solver/CMakeLists.txt` (target `psipp_init_bench`, next to `psipp_feasible_experiment`)
- Create: `experiments/psipp_cluster/build.sh`, `experiments/psipp_cluster/gen_cmds.py`, `experiments/psipp_cluster/run_array.sh`, `experiments/psipp_cluster/aggregate.py`, `experiments/psipp_cluster/nfd_stub/nfd.hpp`

**Interfaces:**
- Produces: `psipp_init_bench <map> <scen> <k> <planner: sipp|psipp> <max_pushes> <seed> <time_limit> <push_cost: inf|number> <closed_per_count: 0|1> <sipp_first: 0|1>` printing one JSON line with the existing fields plus `push_cost`, `closed_per_count`, `sipp_first`, `fail_no_path`, `fail_cycle`, `pushed_replanned`.
- `aggregate.py`: `def load(paths: list[str]) -> list[dict]`, `def summarize(rows: list[dict]) -> list[dict]` grouped by (map, k, variant) with n, success rate, median time, median attempts, mean of each counter.

- [ ] **Step 1: Write the aggregator self-test** `aggregate.py --selftest`: feeds `summarize` one complete success line and one truncated timeout line (`{"map":"x.map",` — unparsable, recorded with the group from its file name) for the same group; asserts n = 2, success rate 0.5, prints `OK`. Each run writes its own file named `<map>_<k>_<variant>_<seed>.json`, so a truncated file still identifies its group.
- [ ] **Step 2: Implement** driver, CMake target, scripts (port from `scratchpad/bench/{build.sh,gen_cmds.py,run_array.sh}`; `gen_cmds.py` emits the full grid from Global Constraints with the baseline map/k list; `run_array.sh` keeps `--partition=compute`, `-c 1`, `--mem=4G`).
- [ ] **Step 3: Verify**: `make psipp_init_bench` builds; one local run on must_push prints valid JSON; `python3 experiments/psipp_cluster/aggregate.py --selftest` prints `OK`.
- [ ] **Step 4: Commit** `git add` the files above `&& git commit -m "Add headless PSIPP benchmark driver and cluster scripts"`

### Task 8: Run the grid on CIIRC and report

- [ ] **Step 1**: rsync the repo source to ciirc via `spark-me`, build with `experiments/psipp_cluster/build.sh` in `~/envs/mapf`.
- [ ] **Step 2**: generate commands, submit the array on `compute`; record the job id.
- [ ] **Step 3**: when done, run `aggregate.py`; report per map and agent count: success rate and median time of plain SIPP vs every PSIPP config, plus the failure-reason split (no path vs cycle). No commit of results unless the user asks.
