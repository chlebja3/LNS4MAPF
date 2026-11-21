# Novelty of a 2025 CTU thesis on anytime MAPF via LNS (literature up to Nov 2026)

## Q1: What is the current state of the art in anytime MAPF via LNS (2021–2026)?

### Takeaway
LNS (MAPF-LNS / MAPF-LNS2 lineage) is still the dominant anytime approach for sum-of-costs on large MAPF instances in 2026. Since 2022, most of the work has gone into **destroy-side adaptivity**: bandit selection of heuristics and neighborhood size (BALANCE 2024), adaptive seed-agent selection (ADDRESS 2025), non-stationary bandits plus new walk heuristics (DyMAB, JAIR 2026). Other directions are parallelism (DROP-LNS 2024) and learned destroy or repair (ML-LNS 2022, Neural Neighborhood Search 2024, LNS2+RL 2025). A careful re-benchmark (Tan et al., SoCS 2025) found that rule-based heuristics remain strong baselines and that learned methods show no clear advantage. LaCAM* is the main non-LNS competitor. It is far better at finding first solutions at extreme scale, but LNS papers consistently report much lower final costs.

### Cited Findings
**Foundations (pre-2022 / 2022)**
- Li, Chen, Harabor, Stuckey, Koenig, "Anytime Multi-Agent Path Finding via Large Neighborhood Search", IJCAI 2021 (extended abstract at AAMAS 2021). It uses three destroy heuristics (random, agent-based, map/intersection-based) chosen by an adaptive roulette wheel, with PP repair. — [AAMAS 2021 abstract PDF](https://www.ifaamas.org/Proceedings/aamas2021/pdfs/p1581.pdf)
- How the agent-based heuristic works. It greedily takes the agent with maximum delay as the seed, using a tabu list that is reset when all agents have been used or the seed has zero delay. A random walk from a random point on the seed's path then collects up to N−1 agents whose paths it crosses; these are treated as the agents causing the delay. — described in [Phan et al. 2025, arXiv 2408.02960v2](https://arxiv.org/html/2408.02960v2)
- MAPF-LNS2 (Li, Chen, Harabor, Stuckey, Koenig, AAAI 2022) applies LNS to *finding* a feasible solution: it destroys and repairs to minimize collisions, and introduces SIPPS (SIPP with soft constraints) for fast single-agent replanning. — summarized in [Sow et al., JAIR 2026](https://www.jair.org/index.php/jair/article/download/19324/27343); code at [github.com/Jiaoyang-Li/MAPF-LNS2](https://github.com/Jiaoyang-Li/MAPF-LNS2)
- Huang, Li, Koenig, Dilkina, "Anytime Multi-Agent Path Finding via Machine Learning-Guided Large Neighborhood Search", AAAI 2022. Imitation learning picks which of several candidate neighborhoods to destroy. — [PDF](https://idm-lab.org/bib/abstracts/papers/aaai22d.pdf)

**Adaptive operator / neighborhood selection**
- Phan, Huang, Dilkina, Koenig, "Adaptive Anytime MAPF Using Bandit-Based LNS" (BALANCE), AAAI 2024 (vol. 38 no. 16). A bi-level multi-armed bandit (UCB1 or Thompson sampling) selects both the destroy heuristic and the neighborhood size. — [arXiv 2312.16767](https://arxiv.org/pdf/2312.16767); [ACM DL](https://dl.acm.org/doi/10.1609/aaai.v38i16.29701)
- Phan, Zhang, Chan, Koenig, "Anytime Multi-Agent Path Finding with an Adaptive Delay-Based Heuristic" (ADDRESS), AAAI 2025 (vol. 39 no. 22). It argues that the stationary agent-based heuristic is a bottleneck that heuristic selection alone cannot fix.
  - Method: agents are sorted by delay, restricted to the top-K (K=32), and the seed is drawn by Thompson sampling over per-agent Beta(α,β). The reward is binary (cost improved or not). The neighborhood is then built with the *same* random walk as MAPF-LNS (N=8).
  - Results: at least 50% cost improvement over MAPF-LNS and other SOTA methods with up to 1,000 agents. At 60 s on Den520d it reports 2,646 SoD vs 33,604 for LaCAM*. On the Random map it is worse than the map-based heuristic. — [arXiv 2408.02960v2](https://arxiv.org/html/2408.02960v2); [AAAI OJS](https://ojs.aaai.org/index.php/AAAI/article/view/34495)
- Sow, de Leng, Wzorek, Heintz (Linköping), "Adaptive Neighborhood Selection for MAPF via Non-Stationary Bandits" (DyMAB), JAIR vol. 86, art. 39, July 2026, doi 10.1613/jair.1.19324.
  - Method: sliding-window non-stationary bandits (α-UCB, ε-greedy) over three destroy heuristics, two of them new:
    - H_sync, a *synchronized random walk* from several seed agents drawn from the top-k most delayed. The walk is constrained by t+1+h(x) < |P_i| and has stagnation resets. The paper says it "balances between the most delayed agents and those blocking their progress".
    - H_entr, an entropy-guided walk toward congested areas.
  - Results: up to 50% lower AUC and 30% lower SoD. It claims to beat MAPF-LNS, MAPF-LNS2, BALANCE, ADDRESS, LaCAM and LaCAM*, with up to 3,500 agents on large city maps. ADDRESS still has faster early convergence (lower AUC) on Boston. — [JAIR PDF](https://www.jair.org/index.php/jair/article/download/19324/27343)

**Parallel LNS**
- Chan, Chen, Lin, Zhang, Harabor, Huang, Koenig, Phan, "Anytime MAPF using Operation Parallelism in LNS" (DROP-LNS), AAMAS 2024 extended abstract. Pairs of destroy and repair operations run concurrently on multicore hardware; two parallelism variants are proposed. — [arXiv 2402.01961](https://arxiv.org/abs/2402.01961); [ACM DL](https://dl.acm.org/doi/10.5555/3635637.3663101). A 2025 IDM Lab abstract also appears to cover DROP-LNS ([mrs25.pdf](https://idm-lab.org/bib/abstracts/papers/mrs25.pdf); not verified).

**Learning-based LNS**
- Yan & Wu (MIT), "Neural Neighborhood Search for Multi-agent Path Finding", ICLR 2024. A convolution and attention network scores subproblems. Each decision must take under about 0.05 s. It reports speedups over SOTA LNS. — [ICLR proceedings](https://proceedings.iclr.cc/paper_files/paper/2024/hash/d41f8403e9bb5141bc2c81fad7658185-Abstract-Conference.html); [code](https://www.github.com/mit-wu-lab/mapf_neural_neighborhood_search)
- Wang, Duhan, Li, Sartoretti, "LNS2+RL", AAAI 2025 (vol. 39 no. 22, pp. 23343–23350). A MARL low-level planner replaces PP in early iterations of LNS2 (the feasibility phase), then the method switches back to PP. It improves success rate over LNS2. — [AAAI OJS](https://ojs.aaai.org/index.php/AAAI/article/view/34501); [arXiv 2405.17794](https://arxiv.org/abs/2405.17794v3)
- Tan, Luo, Li, Ma, "Reevaluation of Large Neighborhood Search for MAPF: Findings and Opportunities", SoCS 2025, pp. 212–220. arXiv v1 was titled "Benchmarking LNS for MAPF".
  - Pitfalls in prior work: baselines that perform below their true level, no unified evaluation setting, and no runnable models for the learned methods.
  - Findings: "rule-based heuristics serve as strong baselines" and learned methods show no clear advantage.
  - Open opportunities: "targeting high-delayed agents, applying contextual algorithms, optimizing replan order and neighborhood size".
  - Code: [github.com/ChristinaTan0704/mapf-lns-unified](https://github.com/ChristinaTan0704/mapf-lns-unified). — [arXiv 2407.09451](https://arxiv.org/abs/2407.09451); [SoCS OJS](https://ojs.aaai.org/index.php/SOCS/article/view/35996)
- Survey: "Where Paths Collide: A Comprehensive Survey of Classic and Learning-Based MAPF" (2025) covers LNS and learned destroy operators. — [arXiv 2505.19219](https://arxiv.org/pdf/2505.19219)
- MAPF-GPT (AAAI 2025) is a learned *decentralized* solver. It is compared against learned solvers, not against anytime LNS. — [arXiv 2409.00134](https://arxiv.org/html/2409.00134v5)

**Competing non-LNS anytime solvers**
- Okumura, "Improving LaCAM for Scalable Eventually Optimal MAPF" (LaCAM*), IJCAI 2023. — [IJCAI](https://ijcai.org/proceedings/2023/28)
- Okumura, "Engineering LaCAM*: Towards Real-Time, Large-Scale, and Near-Optimal MAPF", AAMAS 2024. It notes that vanilla LaCAM* refinement is "notably weak" and improves it with dynamic incorporation of alternative solutions, recursive LaCAM*, Monte-Carlo successor generation and dispersed precomputed paths. This refinement is not LNS. — [AAMAS 2024 PDF](https://aamas.csc.liv.ac.uk/Proceedings/aamas2024/pdfs/p1501.pdf)
- Shen, Zhang, Chen, Harabor, "A Lightweight Traffic Map for Efficient Anytime LaCAM*", arXiv March 2026. It calls LaCAM* "the current state of the art" and builds a dynamic traffic map during the search. It does not compare to LNS. — [arXiv 2603.07891](https://arxiv.org/abs/2603.07891)
- DyMAB (JAIR 2026) reports that at 3,500 agents LaCAM* behaves like LaCAM, "because the initial solution search exhausts the majority of the time budget". It finds LaCAM and LaCAM* costs much higher than LNS. — [JAIR PDF](https://www.jair.org/index.php/jair/article/download/19324/27343)
- Newer anytime variants for other MAPF models:
  - LSRP* (asynchronous actions), arXiv Nov 2026. — [arXiv 2611.10363](https://arxiv.org/abs/2611.10363)
  - AOC-CBS (continuous time), arXiv 2026. — [arXiv 2608.08175](https://arxiv.org/pdf/2608.08175)
  - SIPP-PP-LNS for real-time multi-robot planning, IJCAI 2026. — [IJCAI 2026/842](https://ijcai.org/proceedings/2026/842)
- LNS for a related problem: "LNS for Multi-Agent Task Assignment and Path Finding with Precedence Constraints" (arXiv 2603.28968, 2026) uses Low-Slack, Agent-Conflict and Failure-Recovery destroy families with ALNS selection. — [arXiv](https://arxiv.org/html/2603.28968)

### Inferences
- Anytime LNS is still SOTA for final SoC on standard benchmarks. LaCAM* is SOTA for scalability and first solutions. A modern evaluation should include **MAPF-LNS (with the adaptive roulette), MAPF-LNS2, BALANCE, ADDRESS, ideally DyMAB, and LaCAM* or Engineering-LaCAM* (lacam3)**. A thesis that compares only against MAPF-LNS/LNS2 would be seen as missing 2024–2026 baselines.
- The field's strongest recent gains come from *which agents* get destroyed (delay-focused seeding) and from *adaptivity* (bandits). Contributions 2 and 3 belong exactly to this space.
- Tan et al. (SoCS 2025) warn that baselines often perform below their true level. This makes the thesis's 2–3x faster re-implementation useful context, but it also means speedup claims should be measured against the official code under identical settings.

### Gaps
- I could not confirm the existence of papers titled "Scaling LNS for MAPF", "MAPF-LNS with learned ... (Tang et al.)", or "LaCAM* + LNS". I found no evidence that they exist, and they are probably conflations of DROP-LNS, ADDRESS and BALANCE.
- I did not access the AAMAS/ICAPS/SoCS 2026 proceedings directly. Unindexed 2026 LNS papers may exist.
- I found no head-to-head comparison of MAPF-GPT with anytime LNS.

## Q2: Has anyone proposed a destroy operator targeting agents whose goals are blocked (goal-blocking / target conflicts)? How does it compare to ADDRESS?

### Takeaway
I found no published MAPF-LNS destroy heuristic that explicitly identifies **goal-blocking** relations (agent A parked on, or passing through, agent B's goal and forcing B to wait) and builds the neighborhood from **chains of blockers**. The closest prior work is delay-based seeding plus a random walk over path crossings (MAPF-LNS agent-based heuristic, ADDRESS) and DyMAB's multi-seed synchronized walk. These capture blocking only *implicitly*. Goal-blocking itself is recognized as a key failure mode of PP (SIPP-PP's goal reservation, IJCAI 2026; AOC-CBS's discussion of parked agents), which supports the motivation for contribution 2.

### Cited Findings
- **MAPF-LNS agent-based heuristic.** Seed = max-delay agent; neighbors = agents met by a random walk along the seed's path. The walk "collects agents whose paths it crosses … treated as contributing to the seed's delay". There is no explicit goal-occupancy test. — [arXiv 2408.02960v2](https://arxiv.org/html/2408.02960v2)
- **ADDRESS.** Changes only the seed selection (Thompson sampling over the top-K delayed agents) and keeps the same random walk. Its text does not mention goal blocking or agents waiting at targets. ADDRESS loses to the map-based heuristic on the small Random map. — [arXiv 2408.02960v2](https://arxiv.org/html/2408.02960v2)
- **DyMAB H_sync.** Several delayed seeds walk simultaneously, constrained to stay within each seed's path-length budget, and any agent conflicting at (y, t+1) is added. The paper describes this as covering "those blocking their progress", but it is still walk-based and not a goal-specific blocking graph. — [JAIR 2026](https://www.jair.org/index.php/jair/article/download/19324/27343)
- **Tan et al. (SoCS 2025)** list "targeting high-delayed agents" and "optimizing replan order and neighborhood size" as open opportunities. — [arXiv 2407.09451](https://arxiv.org/abs/2407.09451)
- **Goal blocking in PP.**
  - SIPP-PP (IJCAI 2026) adds "a limited goal reservation strategy to prevent goal-blocking". — [IJCAI 2026/842](https://ijcai.org/proceedings/2026/842)
  - AOC-CBS notes that plain prioritized SIPP fails when an early agent parks on a task vertex and blocks a later agent, and proposes tier-prioritized repair. — [arXiv 2608.08175](https://arxiv.org/pdf/2608.08175)
- **In a related problem (TAPF-PC, 2026)**, the destroy operators target low-slack precedence edges and agent conflicts, not goal blocking. — [arXiv 2603.28968](https://arxiv.org/html/2603.28968)

### Inferences
- **Contribution 2 (blocked operator): novel as a specific operator.** The trigger (goal occupied by another agent, causing waiting) and the construction (transitive blocker chains, then random-walk fallback, with an adaptive mechanism) are not in MAPF-LNS, BALANCE, ADDRESS or DyMAB.
- **Conceptual overlap is real.** All of these target "agents that cause delay". Reviewers will ask how the blocked operator differs from ADDRESS/DyMAB seeding. The strongest framing is that it encodes a *causal, structural* delay cause (goal occupation) instead of a random walk over path crossings. It also composes naturally with ADDRESS's Thompson-sampled seeds or with BALANCE/DyMAB as an extra arm.
- The thesis claims "faster early convergence", which corresponds to a lower AUC. ADDRESS and DyMAB report AUC, so the comparison should be made on AUC with their benchmark maps (Den520d, warehouse, Paris/city, ost003d, random-32-32-20) and agent counts (700–1,000).
- To make the novelty claim safe, run an ablation that plugs the blocked operator into BALANCE or ADDRESS as an additional arm. The 2024–2026 literature evaluates operators that way.

### Gaps
- I did not inspect the ML-LNS (Huang 2022) features or the Neural Neighborhood Search features in detail. Learned models might implicitly use goal-occupancy features, but they do not define such an operator.
- I did not have full-text access to every 2026 venue.

## Q3: SIPP improvements — the path-length upper bound (free cells after the last dynamic obstacle), pruning, tie-breaking, and bounded-suboptimal SIPP as repair

### Takeaway
- **Bounded-suboptimal and anytime SIPP are well established:** Narayanan et al. 2012; Yakovlev, Andreychuk and Stern, ICAPS 2020; and Rybecký, Kulich, Andreychuk and Yakovlev, SoCS 2021, which includes CTU authors.
- **The static-timestep idea already exists in MAPF-LNS2's code:** after the last constraint, the world is static, so the search can collapse time.
- **The free-cell bound appears unpublished as a theorem:** an explicit length bound of T_static + (#free cells) used to terminate or prune SIPP early when no solution exists. It is likely folklore-adjacent.
- **Using bounded-suboptimal single-agent search as the LNS repair to improve global SoC seems new.** I found no paper making that specific use.

### Cited Findings
- Narayanan, Phillips, Likhachev, "Anytime Safe Interval Path Planning for Dynamic Environments", IROS 2012. Weighted or anytime SIPP with a finite-horizon truncation. — [PDF](https://www.cs.cmu.edu/~venkatrn/papers/iros12.pdf)
- Yakovlev, Andreychuk, Stern, "Revisiting Bounded-Suboptimal Safe Interval Path Planning", ICAPS 2020, pp. 300–304, doi 10.1609/icaps.v30i1.6674. It revisits WSIPP, proposes two alternatives (FocalSIPP and a duplicate-allowing variant), and finds no single winner. — [ICAPS OJS](https://ojs.aaai.org/index.php/ICAPS/article/view/6674); [arXiv 2006.01195](https://arxiv.org/pdf/2006.01195)
- Rybecký, Kulich, Andreychuk, Yakovlev, "Towards Narrowing the Search in Bounded-Suboptimal Safe Interval Path Planning", SoCS 2021 (CTU CIIRC). It builds on WSIPP and FocalSIPP. — [PDF](https://imr.ciirc.cvut.cz/uploads/Papers-pdfs/Rybecky21socs.pdf)
- Ali & Yakovlev, "Safe Interval Path Planning With Kinodynamic Constraints", AAAI 2023. Shows that SIPP variants are incomplete under kinodynamic constraints. — [arXiv 2302.00776](https://arxiv.org/pdf/2302.00776)
- **SIPPS (in MAPF-LNS2).**
  - SIPP with soft constraints that minimizes collisions with soft-obstacle paths, with ties broken by length.
  - Uses a `holding_time`: the earliest time the agent can stay at its goal forever.
  - Uses path-max f-values and a `length_max` pruning bound (`next_timestep + h > length_max`).
  - — code [SIPP.cpp](https://github.com/Jiaoyang-Li/MAPF-LNS2/blob/master/src/SIPP.cpp)
- **MAPF-LNS2 space-time A\*.** Computes `static_timestep = constraint_table.getMaxTimestep() + 1 // everything is static after this timestep`. Once next_timestep exceeds it, the search "switch[es] to space A* where we always use the same timestep". This makes the search space finite and terminates on infeasibility. — code [SpaceTimeAStar.cpp](https://github.com/Jiaoyang-Li/MAPF-LNS2/blob/master/src/SpaceTimeAStar.cpp)
- **Termination in prioritized SIPP papers** is stated simply as "OPEN becomes empty". — [Prioritized SIPP with kinematic constraints, arXiv 2108.05145](https://arxiv.org/html/2108.05145)
- **PP theory.**
  - Morag, Zhang, Koyfman, Chen, Felner, Harabor, Stern, "Prioritised Planning with Guarantees", SoCS 2024. Formalizes PC-MAPF, proves it NP-hard, and proposes PCS (complete and optimal for a fixed order). — [SoCS OJS](https://ojs.aaai.org/index.php/SOCS/article/view/31545)
  - Extended version: "Prioritised Planning: Completeness, Optimality, and Complexity", JAIR vol. 84, art. 21, Nov 2025, doi 10.1613/jair.1.19358 (PaPS / PCS / PFCS). — [PDF](https://object.cloud.sdsc.edu/v1/AUTH_da4962d3368042ac8337e2dfdd3e7bf3/ml-papers/JAIR/2025/prioritised_planning_completeness_optimality_and_complexity__6edb012b.pdf)
  - A text search of the JAIR full text found no explicit "T_last + |V|" style length bound (my grep for "upper bound", "horizon", "|V|" and "free cell" returned nothing relevant).

### Inferences
- **Upper bound via free cells: partially overlapping / folklore.** The underlying fact is that after the last dynamic constraint all obstacles are static, so a shortest path from then on needs at most |free cells|−1 more steps. Li's space-time A* uses the static-timestep collapse, and SIPP's interval structure is already finite. What may still be novel:
  - stating the bound explicitly;
  - using it as a pruning bound (g+h > T_static + #free) inside SIPP for early infeasibility detection, and measuring the reduction in expansions on failing PP calls.
  Position it as an engineering contribution with a correctness proof, not as a new algorithm.
- **Improved pruning and tie-breaking: probably incremental.** Tie-breaking (e.g., by h or by fewer waits) and dominance pruning in SIPP are common practice. They are presentable as engineering, backed by expansion counts. I found no 2024–2026 paper on SIPP tie-breaking specifically for LNS repair.
- **Bounded-suboptimal SIPP as LNS repair: the single-agent algorithm is published (WSIPP/FocalSIPP); its use as an LNS repair operator that improves global SoC is the novel part.** That improvement can happen because suboptimal single-agent paths leave room for later agents. This also connects to the "optimizing replan order" opportunity in Tan et al. (SoCS 2025). The thesis should cite Yakovlev et al. 2020 and Rybecký et al. 2021, the latter co-authored at CTU.
- **Prior suboptimal repair inside LNS.** MAPF-LNS2 and LNS2+RL replace or relax the repair, but only for *feasibility* (collision minimization), not for SoC optimization. The original MAPF-LNS also evaluated CBS-style repairs alongside PP; I did not verify the details in this pass.

### Gaps
- I could not find a primary source that states the "T_static + #free cells" bound as a named lemma. I cannot rule out that it appears in a textbook or thesis.
- I did not verify the exact repair-operator variants of the original MAPF-LNS paper (PP vs. CBS vs. EECBS) from its full text.

## Q4: Are LNS solvers still SOTA in 2025–2026, and what is the per-contribution verdict?

### Takeaway
Yes. For the anytime SoC objective, 2025–2026 papers (ADDRESS at AAAI 2025, DyMAB at JAIR 2026) still report LNS beating LaCAM/LaCAM* by large margins in final cost. LaCAM-family papers present themselves as SOTA for scalability and first-solution speed. Learned LNS has not clearly beaten rule-based LNS (Tan et al., SoCS 2025).

### Cited Findings
- ADDRESS vs. LaCAM* at 60 s (sum of delays): Random 3,343 vs 10,146; Ost003d 10,788 vs 38,632; Den520d 2,646 vs 33,604; Warehouse 3,048 vs 70,108; City 2,645 vs 48,761. — [arXiv 2408.02960v2](https://arxiv.org/html/2408.02960v2)
- DyMAB: "LaCAM and LaCAM* serve as reference points … their higher costs confirm the advantage of anytime LNS methods at this scale." — [JAIR 2026](https://www.jair.org/index.php/jair/article/download/19324/27343)
- LaCAM* is called "the current state of the art" by Shen et al. (2026), in a paper that compares only within the LaCAM family. — [arXiv 2603.07891](https://arxiv.org/abs/2603.07891)
- "Rule-based heuristics serve as strong baselines … learning-based methods show no clear advantage." — [Tan et al. SoCS 2025](https://arxiv.org/abs/2407.09451)

### Inferences — verdict per contribution
| Contribution | Verdict | Closest prior work | Positioning advice |
|---|---|---|---|
| 1a. SIPP path-length upper bound (free cells after last dynamic obstacle) | Partially overlapping / folklore | static_timestep collapse in MAPF-LNS2 space-time A* ([code](https://github.com/Jiaoyang-Li/MAPF-LNS2/blob/master/src/SpaceTimeAStar.cpp)); SIPP's finite interval graph | Present as a formal lemma plus an engineering speed-up for *failing* PP calls; quantify expansions saved |
| 1b. Improved node pruning, tie-breaking, heuristics in SIPP | Incremental / engineering | SIPPS (MAPF-LNS2), WSIPP/FocalSIPP literature | Show ablations of expansions and SoC; don't oversell |
| 1c / 3. Bounded-suboptimal SIPP as LNS repair | Algorithm known; **use as SoC-improving LNS repair appears novel** | Narayanan 2012; Yakovlev et al. ICAPS 2020; Rybecký et al. SoCS 2021; LNS2+RL (learned repair, feasibility only) | Frame it as "single-agent suboptimality that yields global SoC gains" and relate it to the replan-order opportunity in Tan et al. 2025 |
| 2. "Blocked" destroy operator (goal-blocking chains, random-walk fallback, adaptive) | **Novel as a specific operator**; conceptually adjacent to delay-based seeding | MAPF-LNS agent-based; ADDRESS (AAAI 2025); DyMAB H_sync (JAIR 2026); goal-blocking motivation in SIPP-PP (IJCAI 2026) | Must compare against ADDRESS and BALANCE (ideally DyMAB) on AUC; test it as a bandit arm |
| 4. Re-implementation (2–3x faster) plus visualizer | Not a scientific novelty; a useful engineering artifact | Unified codebase of Tan et al. 2025 (mapf-lns-unified) | Benchmark against official code under identical settings; consider open-sourcing |

- **Overall:** the thesis remains relevant and partly novel as of late 2026. The main risk is evaluation that omits the 2024–2026 baselines: BALANCE, ADDRESS, DyMAB, and Engineering-LaCAM*.

### Gaps
- The thesis text was not available to me, so the verdicts rest on the contribution descriptions I was given.
- Some 2026 proceedings (AAMAS, ICAPS and SoCS 2026) were not exhaustively searched.
