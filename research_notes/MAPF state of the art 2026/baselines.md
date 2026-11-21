# MAPF baselines for a late-2026 PSIPP (PP-with-pushing init) + anytime LNS paper

Research date: 2026-11-21. Repo metadata was pulled with the GitHub API (`gh api repos/...`) on this date. "Verified" means I read a primary source (paper, repo README, or license file) in this session. Items marked [background, verify] come from my prior knowledge and were not re-checked in this session. Treat them as leads, not as citable facts.

---

## Q1. Feasibility / initial-solution SOTA: which solvers matter and what they claim

### Takeaway
The LaCAM family is the main SOTA for fast, scalable feasible solutions: LaCAM (AAAI-23), LaCAM* (IJCAI-23), and lacam3, the "Engineering LaCAM*" code (AAMAS-24). The newest 2025–2026 additions build on LaCAM: LG-LaCAM, AAAI-26, adds local guidance, and Goal-Branching LaCAM (arXiv, Nov 2026) targets dense instances. MAPF-LNS2 (AAAI-22) is the standard PP/LNS-family feasibility baseline, and it is the closest relative of PSIPP. One important 2026 finding: on very dense bottleneck maps (60–90% of free cells occupied), LaCAM* and LaCAM3 solve only 44% and 33% of instances within 30 s. That is exactly the regime where a PP-with-pushing method could win.

### Cited Findings

**LaCAM / LaCAM* / lacam3 (Okumura)**
- LaCAM: K. Okumura, "LaCAM: Search-Based Algorithm for Quick Multi-Agent Pathfinding", AAAI 2023. — [arXiv 2211.13432](https://arxiv.org/abs/2211.13432), [AAAI OJS](https://ojs.aaai.org/index.php/AAAI/article/view/26377/26149)
- LaCAM*: K. Okumura, "Improving LaCAM for Scalable Eventually Optimal Multi-Agent Pathfinding", IJCAI 2023. — [arXiv 2305.03632](https://arxiv.org/pdf/2305.03632)
- lacam3 repo: it accompanies "Engineering LaCAM*: Towards Real-Time, Large-Scale, and Near-Optimal Multi-Agent Pathfinding" (AAMAS-24). The README says LaCAM(*) "can address large instances, e.g., with more than 1k agents", describes the code as the third generation after lacam (AAAI-23) and lacam2 (IJCAI-23), and says a Python binding (pybind branch) was added in Mar 2025. The code is C++17 built with CMake, and the CLI is `build/main -i <scen> -m <map> -N <agents>`, so it reads MovingAI .map/.scen directly and writes `build/result.txt`. It is MIT licensed. — [Kei18/lacam3 README](https://github.com/Kei18/lacam3)
- Repo metadata as of 2026-11-21: lacam3 is C++, MIT, last push 2026-10-11 (actively maintained). lacam is C++, MIT, last push 2025-08-22. lacam2 is C++, MIT, last push 2025-08-22. pibt2 is C++, MIT, last push 2025-04-14. — [GitHub API: Kei18/lacam3](https://github.com/Kei18/lacam3), [Kei18/lacam](https://github.com/Kei18/lacam), [Kei18/lacam2](https://github.com/Kei18/lacam2), [Kei18/pibt2](https://github.com/Kei18/pibt2)
- Okumura's talk and slides are titled "Pathfinding for 10k agents", which supports the 10k-agent scalability claim for LaCAM*. — [lacam3 README links](https://github.com/Kei18/lacam3)
- Independent 2026 evidence (GRACE benchmark): LaCAM reached 100% success over the whole sweep on four grid maps and kept 100% up to N=2500 in an extended stress test with a 1-min budget. MAPF-LNS2 degraded more gradually as the agent count grew. — [GRACE, arXiv 2603.10858](https://arxiv.org/pdf/2603.10858) (search-snippet level, not read in full)
- **Dense-regime weakness of LaCAM (key for PSIPP).** Goal-Branching LaCAM (S. Chang, D. Harabor, arXiv 2611.07528, submitted 5 Nov 2026, no venue yet) tested 32×32 MovingAI maps at 60–90% occupancy of free cells, with 25 generated scenarios × 5 seeds, a 30 s limit, 5 GB memory, and one thread per run on a Ryzen 5 7600X. Success on dense bottleneck maps:
  - maze-32-32-2: LaCAM* 30%, LaCAM3 28%, GB 86–87%
  - maze-32-32-4: LaCAM* 53%, LaCAM3 27%, GB 80%
  - room-32-32-4: LaCAM* 50%, LaCAM3 45%, GB 100%
  - Overall: LaCAM* 44%, LaCAM3 33%, GB 89%
  - On open maps (random-32-32-10/20, empty-32-32, 70–95% density) all solvers solved 98–100%.
  - On maze-128-128-1 in the full benchmark: GB 121/125, LaCAM* 80, LaCAM3 69.
  - GB's first-solution cost is about 15–22% higher than LaCAM* on bottleneck maps.
  - The authors say LaCAM3 is slower than LaCAM* on every instance because of its upfront guide-path computation. LaCAM3 was run with `--no-star --no-multi-thread`.
  - Code: github.com/Elysium8/gb-lacam2. — [arXiv 2611.07528 (HTML)](https://arxiv.org/html/2611.07528), [abs](https://arxiv.org/abs/2611.07528)
- LaGAT ("Graph Attention-Guided Search for Dense Multi-Agent Pathfinding", arXiv 2510.17382) reports that in one dense case LaCAM3 reached only 75% success in 30 s, and that LNS2 failed outright at 128 agents on its dense warehouse tests (30 s timeout). — [arXiv 2510.17382](https://arxiv.org/pdf/2510.17382) (search-snippet level)

**LaCAM with guidance (2025–2026)**
- LG-LaCAM: T. Arita, K. Okumura, "Local Guidance for Configuration-Based Multi-Agent Pathfinding", AAAI 2026 (arXiv 2510.19072). It extends LaCAM with agent-wise short-horizon spatiotemporal hints. The README claims it "reduces flowtime by up to 38% over baseline LaCAM and outperforms state-of-the-art anytime solvers such as LaCAM3 and LNS2" for up to 1,000 agents. Code: C++17, MIT, `--lg --lg_window 20`, MovingAI .map/.scen input. — [allegorywrite/lg_lacam](https://github.com/allegorywrite/lg_lacam), [AAAI OJS](https://ojs.aaai.org/index.php/AAAI/article/view/40169)
- LG-LaCAM protocol: MovingAI "random" scenarios, 25 instances per map and agent count, up to 1,000 agents, 30 s limit with unsolved runs counted as failures. A scalability test used 300 s and up to 10k agents on warehouse-20-40-10-2-2.
  - Maps: empty-48-48, random-64-64-20, den312d, maze-128-128-10, ost003d, lak303d, Boston_0_256, Paris_1_256, and two warehouse maps.
  - Baselines: LaCAM, GG (global guidance via SUO), LNS2, and lacam3 (anytime). Cost is flowtime normalized by the sum of shortest-path distances.
  - LaCAM-based solvers solved everything in 30 s. LNS2 failed on den312d with 800 and 1,000 agents. LG+LNS is comparable to lacam3 at the deadline.
  - Hardware: Intel Ultra 9 185H laptop. — [arXiv 2510.19072v2](https://arxiv.org/html/2510.19072v2)
- Lifelong LaCAM with Local Guidance (LLLG): Arita & Okumura, SoCS 2026 (arXiv 2605.16855). It is lifelong only, and its abstract calls local guidance "state-of-the-art for one-shot MAPF". It is not a one-shot baseline. — [arXiv 2605.16855](https://arxiv.org/html/2605.16855), [SoCS OJS](https://ojs.aaai.org/index.php/SOCS/article/view/43067)
- Lightweight traffic map for anytime LaCAM* (Shen et al., arXiv 2603.07891, Mar 2026). It builds a dynamic traffic map during LaCAM* search and claims higher quality than guidance-path approaches. — [arXiv 2603.07891](https://arxiv.org/abs/2603.07891) (abstract-level only)
- The "Scaling Lifelong MAPF to More Realistic Settings" paper is H. Jiang, Y. Zhang, R. Veerapaneni, J. Li, SoCS 2024 (arXiv 2404.16162). It is a lifelong-MAPF challenges paper describing the League of Robot Runners 2023 winning entry, not a one-shot baseline. — [arXiv 2404.16162](https://arxiv.org/abs/2404.16162)

**MAPF-LNS2 (closest relative to PSIPP)**
- J. Li, Z. Chen, D. Harabor, P. J. Stuckey, S. Koenig, "MAPF-LNS2: Fast Repairing for Multi-Agent Path Finding via Large Neighborhood Search", AAAI 2022. The repo is a superset of MAPF-LNS (IJCAI 2021): one binary `./lns` covers both the collision-repair LNS2 and the anytime cost-improving LNS. It reads MovingAI .map/.scen and uses the first k rows of the scen. It needs Boost and Eigen, and CI badges show Ubuntu and macOS. — [Jiaoyang-Li/MAPF-LNS2 README](https://github.com/Jiaoyang-Li/MAPF-LNS2)
- License: a USC research license ("educational, research and non-profit purposes"; commercial use requires contacting USC Stevens Center). It is not OSI open source. GitHub reports NOASSERTION. The same license applies to EECBS, and GitHub shows NOASSERTION for MAPF-LNS and PBS too. The last push to MAPF-LNS2 was 2024-05-09, so it is effectively unmaintained but stable. — [MAPF-LNS2 license.txt](https://github.com/Jiaoyang-Li/MAPF-LNS2/blob/master/license.txt), [EECBS license.md](https://github.com/Jiaoyang-Li/EECBS)
- Tan et al. (SoCS 2025) used LNS2 and LaCAM2 as initial solvers because EECBS, PP with random ordering, and PPS do not solve all 25 scenarios within 10 s. They note LaCAM2 "is faster than LNS2, [but] its solution quality is generally worse than LNS2". — [Tan, Luo, Li, Ma, SoCS 2025 PDF](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf)

**Learned solvers**
- MAPF-GPT: Andreychuk et al., "MAPF-GPT: Imitation Learning for Multi-Agent Pathfinding at Scale", AAAI 2025 (arXiv 2409.00134). It is a decentralized transformer policy imitation-trained on about 1B observation-action pairs generated by LaCAM* on maze and random maps, and it is evaluated on the POGEMA benchmark. Repo: CognitiveAISystems/MAPF-GPT, MIT, Python/PyTorch with uv or Docker, last push 2026-04-23. It does not consume MovingAI .scen natively; it uses POGEMA configs. — [arXiv 2409.00134](https://arxiv.org/pdf/2409.00134), [AAAI OJS](https://ojs.aaai.org/index.php/AAAI/article/view/34477/36632), [repo](https://github.com/CognitiveAISystems/MAPF-GPT)
- MAPF-GPT-DDG ("Advancing Learnable MAPF Solvers with Active Fine-Tuning", arXiv 2506.23793). Repo Cognitive-AI-Systems/MAPF-GPT-DDG, Python, MIT, last push 2026-02-21. — [arXiv 2506.23793](https://arxiv.org/pdf/2506.23793), [GitHub API](https://github.com/Cognitive-AI-Systems/MAPF-GPT-DDG)
- PRIMAL3 (arXiv 2608.04905, Aug 2026) is RL/IL "leveraging LaCAM3". Its own evaluation says search-based solvers are stronger: LNS2 reaches 100% success on random maps across all tested agent counts. — [arXiv 2608.04905](https://arxiv.org/abs/2608.04905v1)
- LNS2+RL: AAAI 2025 combines MARL with LNS2. — [AAAI OJS](https://ojs.aaai.org/index.php/AAAI/article/view/34501/36656) (title-level only)
- DiffLNS ("Discrete Diffusion for Complex and Congested MAPF with Sparse Social Attention", arXiv 2605.13296) reports that LaCAM3's success degrades quickly on dense cases (Large Maze, hardest Medium Warehouse) under matched per-instance budgets, and treats MAPF-LNS2 as a strong classical baseline. — [arXiv 2605.13296](https://arxiv.org/html/2605.13296) (search-snippet level)
- The SoCS 2025 reevaluation of LNS concluded that "rule-based heuristics serve as strong baselines, while current learning-based methods show no clear advantage on time efficiency or improvement capacity". — [Tan et al. SoCS 2025](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf)

**Others (background, verify before citing)**
- [background, verify] PIBT: Okumura, Machida, Défago, Tamura, "Priority Inheritance with Backtracking for Iterative Multi-agent Path Finding", IJCAI 2019, journal version in AIJ 2022. pibt2 (MIT, C++, MovingAI input) contains PIBT and PIBT+ for one-shot MAPF. PIBT is incomplete: it is complete only for reachability in biconnected-like graphs, and it can livelock or time out in one-shot MAPF on dense or narrow maps. PIBT+ adds a fallback for that case. — [Kei18/pibt2](https://github.com/Kei18/pibt2)
- [background, verify] EECBS: Li, Ruml, Koenig, "EECBS: A Bounded-Suboptimal Search for Multi-Agent Path Finding", AAAI 2021. Repo C++, USC license, MovingAI input. — [Jiaoyang-Li/EECBS](https://github.com/Jiaoyang-Li/EECBS)
- [background, verify] PBS: Ma, Harabor, Stuckey, Li, Koenig, "Searching with Consistent Prioritization for Multi-Agent Path Finding", AAAI 2019. Repo last push 2025-04-28. — [Jiaoyang-Li/PBS](https://github.com/Jiaoyang-Li/PBS)
- Greedy PBS (GPBS): Chan, Stern, Felner, Koenig, SoCS 2023. — [SoCS OJS](https://ojs.aaai.org/index.php/SOCS/article/download/27278/27051/31322)
- [background, verify] Push and Rotate: de Wilde, ter Mors, Witteveen, JAIR 2014. It is complete for the class of instances with at least 2 empty vertices, but its solutions are known to be very long. No maintained C++ MovingAI implementation was found in this session.
- [background, verify] Traffic Flow Optimisation (TFO) guidance: Z. Chen, D. Harabor, J. Li, P. J. Stuckey, "Traffic Flow Optimisation for Lifelong Multi-Agent Path Finding", AAAI 2024. It targets lifelong MAPF, and LG-LaCAM's "GG" baseline uses SUO global guidance instead.

### Inferences
- For a PSIPP paper focused on dense instances, the decisive comparison is MAPF-LNS2 (same PP/SIPPS family, the incumbent "repair" approach) plus LaCAM-family solvers (the general scalability SOTA). Goal-Branching LaCAM's numbers suggest LaCAM* and LaCAM3 are beatable on dense bottleneck maps. Expect LaCAM to dominate on open maps and on large agent counts where density is moderate.
- LaCAM3 run with defaults uses multiple threads and anytime refinement. For a fair "first solution" comparison, report both the default and `--no-star --no-multi-thread` (as GB-LaCAM did), or report time to first solution.
- Learned solvers (MAPF-GPT, SCRIMP, LaGAT, PRIMAL3) are optional. They need GPU or Python stacks, do not read MovingAI .scen directly (POGEMA), and recent independent evaluations show they do not beat LaCAM or LNS2. One sentence in related work justifying their exclusion is enough.

### Gaps
- I did not verify the exact accepted venue or status of Goal-Branching LaCAM (arXiv only, Nov 2026) or its license.
- I did not read the Engineering LaCAM* paper's tables, so I could not verify its headline success-rate numbers. The project page (kei18.github.io/lacam3) returned empty content to the fetcher.
- No sources fetched for SCRIMP, LaGAT details, "LaCAM + MCTS", or "PP with random restarts" as a standalone published method. PP-RR is usually described as a baseline inside MAPF-LNS2 and related papers, not as a separate paper; I did not confirm a canonical citation.
- I did not check the PIBT/PIBT+ incompleteness wording against the AIJ 2022 paper in this session.

---

## Q2. Anytime solution-quality SOTA

### Takeaway
The anytime-LNS line in 2026 runs MAPF-LNS (IJCAI-21) → BALANCE (AAAI-24, bandit over destroy heuristics and neighborhood sizes) → ADDRESS (AAAI-25, Thompson sampling over the top-K most-delayed agents) → DyMAB (JAIR, Jul 2026, non-stationary bandits). LaCAM*/lacam3 is the main non-LNS anytime competitor, but it is much worse in quality at scale. ADDRESS reports LaCAM* final sum-of-delays 3–20× worse than ADDRESS at 60 s. LG-LaCAM (+LNS) is the newest guidance-based competitor.

### Cited Findings
- **ADDRESS**: "Anytime Multi-Agent Path Finding with an Adaptive Delay-Based Heuristic", AAAI 2025. It is a single-destroy-heuristic MAPF-LNS variant: it sorts agents by delay, applies restricted Thompson sampling over the top-K (K=32) most-delayed agents to pick a seed agent, and builds the neighborhood with MAPF-LNS's random-walk routine, using neighborhood size N=8.
  - Setup: maps random-32-32-20, ost003d, den520d, warehouse-20-40-10-2-2, and Paris_1_256; the 25 public random scenarios per map; a 60 s budget.
  - Baselines: the original MAPF-LNS, MAPF-LNS2, BALANCE (authors' code, defaults), and LaCAM*. All MAPF-LNS variants use PP for the initial solution and for repair.
  - Metrics: sum of delays and AUC.
  - Results: ADDRESS beats the other methods on large maps (up to 1,000 agents on warehouse/city), but not on random-32-32-20, where overhead dominates and the map-based heuristic wins. The table vs LaCAM* at the maximum agent count, 60 s, shows sum of delays: Random 3,344 vs 10,146; Ost003d 10,788 vs 38,632; Den520d 2,646 vs 33,604; Warehouse 3,048 vs 70,108; City 2,645 vs 48,761.
  - Code: github.com/JimyZ13/ADDRESS (C++, based on MAPF-LNS2 and anytime-mapf code, USC-derived license, NOASSERTION; last push 2025-05-28). — [arXiv 2408.02960](https://arxiv.org/pdf/2408.02960), [AAAI OJS](https://ojs.aaai.org/index.php/AAAI/article/download/34495/36650), [JimyZ13/ADDRESS](https://github.com/JimyZ13/ADDRESS)
- **BALANCE**: T. Phan et al., "Adaptive Anytime Multi-Agent Path Finding Using Bandit-Based Large Neighborhood Search", AAAI 2024. It uses a bi-level multi-armed bandit over destroy heuristic × neighborhood size and claims cost improvements of at least 50% over SOTA anytime MAPF at large scale. Code: thomyphan/anytime-mapf (C++, NOASSERTION, last push 2023-12-15). — [arXiv 2312.16767](https://arxiv.org/pdf/2312.16767), [AAAI OJS](https://ojs.aaai.org/index.php/AAAI/article/view/29701/31201), [thomyphan/anytime-mapf](https://github.com/thomyphan/anytime-mapf)
- **DyMAB**: "Adaptive Neighborhood Selection for MAPF via Non-Stationary Bandits", JAIR, listed as published 29 Jul 2026. It is a non-stationary MAB inside ALNS and claims lower sum of delays than recent SOTA. — [JAIR](https://www.jair.org/index.php/jair/article/view/19324) (abstract-level; authors, code and setup not checked)
- **Operation-parallel LNS**: "Anytime Multi-Agent Path Finding using Operation Parallelism in Large Neighborhood Search" (arXiv 2402.01961). It parallelizes destroy and repair across worker threads. — [arXiv 2402.01961](https://arxiv.org/pdf/2402.01961)
- **ML-guided LNS**: Huang, Li, Koenig, Dilkina, "Anytime Multi-Agent Path Finding via Machine Learning-Guided Large Neighborhood Search", AAAI 2022. — [AAAI PDF](https://cdn.aaai.org/ojs/21168/21168-13-25181-1-2-20220628.pdf), [IDM-lab abstract](https://idm-lab.org/bib/abstracts/papers/aaai22d.pdf)
- **SoCS 2025 reevaluation** (J. Tan, Y. Luo, J. Li, H. Ma, "Reevaluation of Large Neighborhood Search for MAPF: Findings and Opportunities"):
  - Reported pitfalls: under-reported or incorrect baseline numbers. Huang et al. 2022's rule-based delays are much higher than Li et al. 2021's, and Phan et al. 2024 imported them. Yan & Wu 2024 used random-32-32-20 results as if they were for random-32-32-10.
  - They also found no unified setting, and no code for the learned methods.
  - Their unified setting uses the same 25 scenarios for all methods, a 10 s cap for the initial solution, LNS2 or LaCAM2 as init, PP as the repair solver, neighborhood sizes {2, 4, 8, 16, 32}, and final delay plus AUC at 60 s and 300 s. Only destroy and repair time is counted.
  - PP beats PBS as the repair solver on larger maps: PBS is better on empty and random maps but far worse on large maps, because PP runs many more iterations.
  - They proposed RandomWalkProb, a RandomWalk variant without the tabu list and with delay-proportional sampling, and found it significantly better.
  - Rule-based heuristics remain strong, and the learned ones (SVM-LNS, Neural-LNS, Bandit-LNS) show no clear advantage.
  - Code: github.com/ChristinaTan0704/mapf-lns-unified. — [SoCS 2025 PDF](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf)
- **LaCAM\*/lacam3 as anytime**: LG-LaCAM reports that LG+LNS is comparable to and sometimes better than lacam3 at the final deadline, and that LG beats lacam3 on initial-solution quality except on warehouse maps. lacam3 was configured with 1–30 random agents per LNS subset and 4 concurrent processes. — [arXiv 2510.19072v2](https://arxiv.org/html/2510.19072v2)
- ADDRESS reports that LaCAM* is clearly outperformed in anytime quality by ADDRESS, MAPF-LNS, MAPF-LNS2 and BALANCE at 60 s. — [arXiv 2408.02960](https://arxiv.org/pdf/2408.02960)

### Inferences
- Credible anytime baselines in 2026: MAPF-LNS (Adaptive, i.e., original ALNS) via the MAPF-LNS2 binary, BALANCE, ADDRESS, and lacam3 with anytime refinement on. All are C++ with MovingAI input and are runnable on Linux. ADDRESS's repo already bundles BALANCE (`--banditAlgo`), which lowers the integration cost.
- Because PSIPP and the new LNS are a re-implementation, also report your own LNS with the original MAPF-LNS destroy set inside your own codebase. This "ablation baseline" separates implementation speed from algorithmic gains. The SoCS 2025 paper explicitly warns about mismatched implementations and time-counting.
- LaCAM init + LNS: Tan et al. 2025 already used LaCAM2 as an alternative init, so "LNS with LaCAM init" is an expected comparison when the PP-family init fails on dense instances.

### Gaps
- I did not verify the exact author list of ADDRESS, BALANCE, or the operation-parallelism paper beyond what is above. My recollection is that ADDRESS is by T. Phan, B. Zhang, S.-H. Chan, S. Koenig [background, verify]. The task's "Chan et al." most likely refers to this paper or to the operation-parallelism paper.
- DyMAB code availability and authors were not checked.
- I found no 2026 AAAI/IJCAI/ICAPS/SoCS anytime-LNS paper beyond DyMAB (JAIR) and LG-LaCAM in this session. The proceedings were not searched exhaustively.

---

## Q3. Typical baselines and benchmark protocol in recent (2024–2026) PP/LNS papers

### Takeaway
The de-facto protocol uses MovingAI maps, the 25 "random" scenarios per map, increasing agent counts that take the first k rows of the scen, 30 s or 60 s limits (300 s for extended runs), success rate for feasibility, and sum-of-delays (SoC minus the sum of shortest paths) plus AUC for anytime quality. Cost is sometimes normalized by the SoC lower bound (sum of shortest-path distances).

### Cited Findings
- ADDRESS (AAAI 2025): five maps (random-32-32-20, ost003d, den520d, warehouse-20-40-10-2-2, Paris_1_256), 25 random scenarios, 60 s, sum of delays and AUC, up to 1,000 agents, on a CentOS cluster with Xeon 2640v4 and 64 GB. — [arXiv 2408.02960](https://arxiv.org/pdf/2408.02960)
- Tan et al. (SoCS 2025): six maps (empty-32-32, random-32-32-20, warehouse-10-20-10-2-1, ost003d, den520d, Paris_1_256), 25 random scenarios, 10 s initial-solution cap, final delay and AUC at 60 s and 300 s, and only destroy and repair time counted. — [SoCS 2025 PDF](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf)
- LG-LaCAM (AAAI 2026): 10 maps, 25 random scenarios, up to 1,000 agents (10k for scalability), 30 s, and success rate plus cost normalized by the sum of shortest-path distances. — [arXiv 2510.19072v2](https://arxiv.org/html/2510.19072v2)
- Goal-Branching LaCAM (arXiv Nov 2026): dense 32×32 maps at 60–95% density with 25 generated scenarios × 5 seeds, plus the full 33-map MovingAI set at 20/40/60/80/100% of the scenario's agents (capped at 1,000; 4,125 runs per solver). It uses a 30 s limit and 5 GB memory, success rate, the geometric-mean speed-up of time to first solution on runs both solvers solved, and SoC divided by the sum of distances. — [arXiv 2611.07528](https://arxiv.org/html/2611.07528)
- The common classic setting (vertex + swap conflicts, stay-at-target, sum of costs equal to the sum of delays plus a constant) is stated explicitly by Tan et al. 2025, and it matches the target paper's setting. — [SoCS 2025 PDF](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf)

### Inferences
- A protocol that matches all four papers: all 33 MovingAI maps, or a representative subset of about 6–10 maps that includes the dense 32×32 maze and room maps where PSIPP should shine. Use the 25 random scenarios, the first-k-agents convention, a 30 s or 60 s limit, single-threaded runs, success rate and time to first solution, then sum of delays (or SoC/LB) and AUC over 60 s or 300 s.
- To claim "dense" wins credibly, adopt GB-LaCAM's dense protocol (60–90% occupancy on maze-32-32-2/4 and room-32-32-4). It is the most recent published dense benchmark, and its LaCAM* and LaCAM3 numbers give a direct reference point.

### Gaps
- I did not extract MAPF-LNS2's original experimental numbers (AAAI 2022) or BALANCE's exact setup in this session.

---

## Q4. Practicality: maintained C++ code that runs on Linux with MovingAI inputs

### Takeaway
Everything needed for a credible classical comparison is C++, runs on Linux, and reads MovingAI .map/.scen natively: lacam/lacam2/lacam3, pibt2, lg_lacam, gb-lacam2, MAPF-LNS2, EECBS, PBS, anytime-mapf (BALANCE), and ADDRESS. The Okumura-line repos are MIT. The Li-line repos (and their forks ADDRESS and BALANCE) are under a USC non-commercial research license. That license is fine for academic comparison but is not OSI open source.

### Cited Findings
| Solver | Repo | Lang | License | Last push | MovingAI input | Source |
|---|---|---|---|---|---|---|
| LaCAM | Kei18/lacam | C++ | MIT | 2025-08-22 | yes | [repo](https://github.com/Kei18/lacam) |
| LaCAM* | Kei18/lacam2 | C++ | MIT | 2025-08-22 | yes | [repo](https://github.com/Kei18/lacam2) |
| Engineering LaCAM* (lacam3) | Kei18/lacam3 | C++17 | MIT | 2026-10-11 | yes (`-i scen -m map -N`) | [repo](https://github.com/Kei18/lacam3) |
| PIBT / PIBT+ | Kei18/pibt2 | C++ | MIT | 2025-04-14 | yes [background, verify] | [repo](https://github.com/Kei18/pibt2) |
| LG-LaCAM | allegorywrite/lg_lacam | C++17 | MIT | 2025-11-11 | yes | [repo](https://github.com/allegorywrite/lg_lacam) |
| Goal-Branching LaCAM | Elysium8/gb-lacam2 | ? | not checked | not checked | uses MovingAI maps | [paper](https://arxiv.org/html/2611.07528) |
| MAPF-LNS / LNS2 | Jiaoyang-Li/MAPF-LNS2 | C++ (Boost, Eigen) | USC research-only | 2024-05-09 | yes (`-m -a -k -t`) | [repo](https://github.com/Jiaoyang-Li/MAPF-LNS2) |
| EECBS | Jiaoyang-Li/EECBS | C++ | USC research-only | 2024-07-24 | yes | [repo](https://github.com/Jiaoyang-Li/EECBS) |
| PBS | Jiaoyang-Li/PBS | C++ | NOASSERTION (USC-style presumed) | 2025-04-28 | yes | [repo](https://github.com/Jiaoyang-Li/PBS) |
| BALANCE | thomyphan/anytime-mapf | C++ | NOASSERTION | 2023-12-15 | yes | [repo](https://github.com/thomyphan/anytime-mapf) |
| ADDRESS (+BALANCE option) | JimyZ13/ADDRESS | C++ (Boost 1.81, Eigen 3.3) | NOASSERTION (USC-derived) | 2025-05-28 | yes | [repo](https://github.com/JimyZ13/ADDRESS) |
| Unified LNS eval | ChristinaTan0704/mapf-lns-unified | not checked | not checked | not checked | yes | [SoCS 2025](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf) |
| MAPF-GPT | CognitiveAISystems/MAPF-GPT | Python/PyTorch | MIT | 2026-04-23 | no (POGEMA) | [repo](https://github.com/CognitiveAISystems/MAPF-GPT) |
| MAPF-GPT-DDG | Cognitive-AI-Systems/MAPF-GPT-DDG | Python | MIT | 2026-02-21 | no (POGEMA) | [repo](https://github.com/Cognitive-AI-Systems/MAPF-GPT-DDG) |

Repo languages, licenses and push dates come from the GitHub API on 2026-11-21. GitHub labels MAPF-GPT's primary language "C++", probably because of a bundled C++ expert solver, but the README shows a Python/uv/PyTorch workflow.

### Inferences
- **Cost-accounting caveat for LaCAM** [background, verify in code]: LaCAM-family `result.txt` reports several cost fields (e.g., `soc`, `sum_of_loss`, `makespan`, with `soc_lb`). One of them counts waits at the goal until the agent finally stops, which is the classic stay-at-target SoC. Another may count up to makespan. Also, LaCAM* optimizes the objective selected by a flag, which may default to something other than SoC in some versions. Always recompute SoC yourself from the output `solution=` paths under exactly your definition, and validate with your own conflict checker (vertex + swap). LG-LaCAM explicitly defines flowtime as travel time "until it stops at the target location". — [arXiv 2510.19072v2](https://arxiv.org/html/2510.19072v2)
- MAPF-LNS2 counts SoC under the same stay-at-target convention as the target paper. Its `-t` limit covers init and improvement together, so to compare "initial solution" times, use its stats or output for the time when the first feasible solution was found.
- lacam3 defaults to multi-threading. Pin it to one thread (`--no-multi-thread`), or report the thread counts, for fairness against single-threaded PSIPP.

### Gaps
- I did not compile or run any of these in this session. The Linux build status is inferred from CMake and Ubuntu CI badges (MAPF-LNS2 has an Ubuntu CI workflow; lacam3 has CI).
- I did not inspect the exact field names in lacam3's `result.txt` or its objective flag defaults in this session.

---

## Q5. Recommended minimal-but-credible baseline sets

### Takeaway
(1) **Feasibility / first solution:** MAPF-LNS2 (init+repair), LaCAM* (lacam2), lacam3 (first-solution mode), and PP-RR implemented in your own codebase (PSIPP without pushing, i.e., plain SIPP-PP with restarts). Optional additions are LG-LaCAM or Goal-Branching LaCAM for dense maps, and PIBT+ as a fast incomplete reference. (2) **Anytime quality:** MAPF-LNS (Adaptive ALNS via the LNS2 binary), BALANCE, ADDRESS, and lacam3 anytime. Optional additions are LG-LaCAM+LNS and your own re-implementation with the original destroy operators as an ablation.

### Cited Findings (justification)
- MAPF-LNS2 is the de-facto PP/LNS feasibility baseline. Tan et al. use it as the default init and every anytime-LNS paper compares to it. — [SoCS 2025](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf), [ADDRESS](https://arxiv.org/pdf/2408.02960)
- LaCAM*/LaCAM3 are the scalability SOTA. Reviewers expect them, and in 2025–2026 papers (LG-LaCAM AAAI-26, GB-LaCAM 2026) they are the reference. Their documented dense-regime weakness (44% and 33% success on dense bottleneck maps) gives PSIPP a clear opportunity. — [arXiv 2611.07528](https://arxiv.org/html/2611.07528), [arXiv 2510.19072v2](https://arxiv.org/html/2510.19072v2)
- BALANCE and ADDRESS are the most recent peer-reviewed anytime-LNS SOTA (AAAI-24 and AAAI-25) and have public C++ code. ADDRESS's repo can run BALANCE. — [JimyZ13/ADDRESS](https://github.com/JimyZ13/ADDRESS), [anytime-mapf](https://github.com/thomyphan/anytime-mapf)
- Learned methods are not needed as primary baselines. The reevaluation finds no clear advantage for them, PRIMAL3's own evaluation finds the search-based solvers stronger, and they require POGEMA rather than MovingAI. — [SoCS 2025](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf), [PRIMAL3](https://arxiv.org/abs/2608.04905v1)

### Inferences
- **Feasibility comparison, recommended set:**
  1. PP-RR (SIPP, random restarts) as the in-house, no-push ablation.
  2. MAPF-LNS2 as the closest published competitor.
  3. LaCAM* (lacam2) single-threaded.
  4. lacam3 with `--no-star --no-multi-thread` for the first solution, plus defaults if threads are reported.
  5. Optionally, GB-LaCAM, because it is the newest dense-specific SOTA; if PSIPP is competitive with it on maze-32-32-2/4 and room-32-32-4, that is a strong result.
  6. Optionally, PIBT+ as a speed reference, noted as incomplete.
  - EECBS and PBS can be skipped or mentioned briefly. Tan et al. report they (and PP, PPS) fail to solve all scenarios within 10 s at scale.
  - Push and Rotate is historically relevant to "pushing" and worth discussing in related work. No maintained MovingAI implementation was found, so treat it as optional.
- **Anytime comparison, recommended set:**
  1. MAPF-LNS (Adaptive) from the MAPF-LNS2 binary.
  2. BALANCE.
  3. ADDRESS.
  4. lacam3 with anytime refinement, which is the main non-LNS anytime method and the one LG-LaCAM compares against.
  5. Your re-implementation with the original operators as an ablation.
  - Start every LNS from the same initial solution where possible (or report init time separately, per Tan et al.). Report sum of delays (or SoC/LB) and AUC at 60 s, optionally also at 300 s.
- Mention MAPF-GPT, MAPF-GPT-DDG, SCRIMP, LaGAT, DiffLNS, PRIMAL3, LNS2+RL, DyMAB, the operation-parallel LNS, and the TFO/SUO guidance work in related work only, unless reviewers specifically target them.

### Gaps
- I did not check whether GB-LaCAM's code is public, licensed, and buildable, so its practicality as a baseline is unverified.
- I did not confirm whether DyMAB has public code. If it does, it would be the newest anytime-LNS baseline (JAIR 2026).
