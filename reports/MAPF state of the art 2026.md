# Pushing inside SIPP still finds open ground

As of November 2026, the work is still publishable, but nothing in it is novel at the level of a concept. Every idea has a close neighbour in the literature. PSIPP's general mechanism has been published several times: a low-level search may displace another agent, and the displaced agent is then replanned recursively. It appears in the PIBT family (PIBT, EPIBT at AAAI 2026, MD-PIBT at IROS 2026) and, most dangerously, in **PUSH (Sanjay & Li, arXiv, August 2026)**, a windowed space-time A* for lifelong MAPF with recursive displacement. No paper found does PSIPP's exact combination. That combination is one-shot, full-horizon prioritized planning (PP) in which a **SIPP** search lets the agent being planned push **already-committed higher-priority** agents (up to about 5, each assumed to wait in its previous cell), replans them recursively, and restarts PP on failure. It is also used as the initializer for an LNS anytime solver. On the thesis side, the "blocked" destroy operator (goal-blocking chains) appears novel. Using bounded-suboptimal SIPP as a cost-improving LNS repair operator also appears new as an application. The free-cell path-length bound is close to folklore, and the pruning and heuristic changes and the 2–3x faster re-implementation are engineering contributions, not scientific ones. The timing is good. In 2026 the newest dense-instance results show LaCAM* and LaCAM3 solving only **44% and 33%** of very dense bottleneck instances in 30 s. That is the regime a PP-with-pushing method is built for. The main risk is not novelty. It is an evaluation that leaves out the 2024–2026 baselines: MAPF-LNS2, LaCAM*/lacam3, BALANCE, ADDRESS, and ideally Goal-Branching LaCAM and DyMAB. Realistic venues are SoCS, ICAPS, AAMAS or ICRA/IROS/RA-L. AAAI or IJCAI would need clear wins over LaCAM* and LNS2.

## PUSH (August 2026) is PSIPP's closest rival, and it differs in six ways

The closest prior work is **PUSH ("Path Updates over Staggered Horizons")** by V. Sanjay and J. Li, posted to arXiv on 7 August 2026 with no venue yet. In PUSH, a modified windowed space-time A* records the agent it displaces in the search state. Each path may displace **at most one** agent, and that agent is replanned recursively while a protected set of higher-priority agents grows, which bounds the recursion. When planning fails, PUSH restores the old path and backtracks to the previous agent in the chain. It uses windows of W=10 and scales to roughly 10k agents ([arXiv](https://arxiv.org/abs/2608.06702), [HTML](https://arxiv.org/html/2608.06702v1)).

It differs from PSIPP in six concrete ways:

- **Setting.** PUSH is lifelong and windowed. PSIPP is one-shot, with full-horizon paths that minimise sum of costs.
- **Search.** PUSH uses space-time A*. PSIPP uses SIPP intervals.
- **Priority direction.** In PUSH a higher-priority agent displaces a lower-priority one, as in PIBT. In PSIPP the agent being planned pushes agents that are already planned and have **higher** priority, which reverses PP's normal direction.
- **Number of pushes.** PUSH allows one displaced agent per path. PSIPP allows about five.
- **Failure handling.** PUSH backtracks along the chain. PSIPP restarts the whole PP attempt with a new order.
- **Push model.** PUSH replans the displaced agent's window from scratch. PSIPP has an explicit "pushed agent waits in its previous cell" model inside the search.

PUSH cites PIBT, EPIBT, MD-PIBT, Token Passing and LaCAM as its pushing lineage, and no earlier "PP with pushing" paper, which suggests there is no older precedent to worry about ([HTML](https://arxiv.org/html/2608.06702v1)). Because PUSH comes from a central MAPF group and uses the same name, **do not call the method "PUSH"**, cite it, and spell out the differences above.

The PIBT line is the conceptual ancestor reviewers will raise:

- **PIBT** (IJCAI 2019 / AIJ 2022) pushes lower-priority agents through priority inheritance, but plans one timestep at a time ([arXiv](https://arxiv.org/html/1901.11282)).
- **EPIBT** (AAAI 2026) adds multi-action operations over short horizons ([AAAI](https://ojs.aaai.org/index.php/AAAI/article/view/40233)).
- **MD-PIBT** (arXiv March 2026, listed as IROS 2026) searches over agent dependencies and supports multi-step paths that collide with **more than one** agent ([arXiv](https://arxiv.org/abs/2603.23405)). This is the nearest analogue to PSIPP's "up to five pushes".
- **Nagai & Okumura (IJCAI 2026)** is one-shot. It replans agents recursively through priority inheritance, but the replanned agents are *not yet planned*, there is no SIPP, and its PP baseline has no pushing ([IJCAI](https://ijcai.org/proceedings/2026/35)).

The other relevant family repairs the *priority order* instead of pushing:

- **PBS** (AAAI 2019) branches on which of two conflicting agents gets priority ([arXiv](https://arxiv.org/pdf/1812.06356)).
- **PCS and PPR\*** (Morag et al., SoCS 2024; JAIR 2025) give PP completeness and optimality guarantees. PPR\* formalises PP with random restarts ([SoCS](https://ojs.aaai.org/index.php/SOCS/article/view/31545), [JAIR](https://www.jair.org/index.php/jair/article/download/19358/27244)).

PSIPP can be framed as a greedy, non-branching cousin of PBS: it keeps the order and lets the lower-priority agent bend a committed plan locally. The classical push-and-swap and push-and-rotate algorithms also share the vocabulary, but they are rule-based macro-operator methods with poor solution quality (Luna & Bekris, IJCAI 2011; de Wilde et al., JAIR 2014; links not re-verified during research: [Push and Swap](https://www.ijcai.org/Proceedings/11/Papers/059.pdf), [Push and Rotate](https://www.jair.org/index.php/jair/article/view/10882)).

The ordering policy is not a contribution in itself. Longest-first ordering already appears in dense-environment work ([MAPF-HD](https://arxiv.org/html/2509.06374)). A 2026 study finds that learned orderings give only "moderate improvements over random priority ordering", and that random total orders with restarts "perform surprisingly well" ([arXiv 2603.23838](https://arxiv.org/html/2603.23838)). No published head-to-head of longest-first, shortest-first and random restarts for one-shot SIPP-PP at high density was found, so the user's ordering ablation is a small but genuine data point.

The search found one gap that must be checked by hand before submission. A 2025 *Robotics and Autonomous Systems* paper on "elevating priorities" for lifelong multi-AGV planning uses SIPP with priority elevation, and its mechanism could not be read because the page returned 403 ([ScienceDirect](https://www.sciencedirect.com/science/article/abs/pii/S0921889025003926)). The venue sweep was also keyword-based, not a full read of the 2026 proceedings.

## Dense instances are where LaCAM is losing ground in 2026

PSIPP's case is strongest exactly where the 2026 state of the art is weak. **Goal-Branching LaCAM** (Chang & Harabor, arXiv, 5 November 2026) tested 32×32 MovingAI maps at 60–90% occupancy of free cells with a 30 s limit. LaCAM* solved only **30%** of maze-32-32-2 instances and LaCAM3 only **28%**, against 86–87% for Goal-Branching LaCAM. Across the dense bottleneck maps, success was **44% for LaCAM\*, 33% for LaCAM3 and 89% for GB-LaCAM**. On open maps, all solvers solved 98–100% ([arXiv 2611.07528](https://arxiv.org/html/2611.07528)). Other papers point the same way. LaGAT reports LaCAM3 at 75% success in a dense case, with LNS2 failing outright at 128 agents in dense warehouses ([arXiv 2510.17382](https://arxiv.org/pdf/2510.17382)). DiffLNS finds that LaCAM3's success drops quickly on large mazes ([arXiv 2605.13296](https://arxiv.org/html/2605.13296)).

The counter-evidence matters as well. PP and LNS2 also "struggle with many agents" in one-shot AGV settings, while LaCAM+PIBT scales ([IJCAI 2026](https://ijcai.org/proceedings/2026/35)). LaCAM held 100% success up to 2,500 agents on four grid maps in the GRACE stress test ([arXiv 2603.10858](https://arxiv.org/pdf/2603.10858)). **LG-LaCAM** (AAAI 2026) solved every instance within 30 s on ten maps with up to 1,000 agents, and claims up to 38% lower flowtime than LaCAM ([arXiv 2510.19072v2](https://arxiv.org/html/2510.19072v2)).

The likely picture is that LaCAM dominates on open maps and at moderate density with large agent counts, and PSIPP has room to win on narrow, bottleneck-heavy and very dense instances. The paper needs two claims together: PSIPP solves far more instances than SIPP-PP, *and* its first solution has a better sum of costs than LaCAM's. Success rate alone will not convince reviewers.

The initializer slot is contested. **MAPF-LNS2** (AAAI 2022) builds a PP+SIPPS plan that may contain collisions and repairs it with LNS ([PDF](https://idm-lab.org/bib/abstracts/papers/aaai22a.pdf)). **LNS2+RL** (AAAI 2025) uses multi-agent RL in LNS2's feasibility phase ([AAAI](https://ojs.aaai.org/index.php/AAAI/article/view/34501/36656)). A 2026 **discrete-diffusion initializer** argues that initialization quality is what decides LNS2's success on congested instances ([arXiv 2605.13296](https://arxiv.org/html/2605.13296)). Reviewers will ask why PSIPP is better than "PP+SIPPS, then LNS2 repair". A head-to-head on time to first feasible solution is therefore the central experiment. None of the reviewed 2023–2026 one-shot papers improves PP's dense-instance success by changing the low-level planner instead of the ordering, which supports PSIPP's novelty claim.

## The thesis's best contributions are the blocked operator and the suboptimal repair

LNS is still the state of the art for final sum of costs. The newest gains come from destroy-side adaptivity:

- **BALANCE** (AAAI 2024) uses a bandit to choose the destroy heuristic and the neighborhood size ([arXiv](https://arxiv.org/pdf/2312.16767)).
- **ADDRESS** (AAAI 2025) uses Thompson sampling over the top-32 most-delayed agents and keeps MAPF-LNS's random walk ([arXiv](https://arxiv.org/html/2408.02960v2)).
- **DyMAB** (JAIR, July 2026) uses non-stationary bandits and adds a synchronized multi-seed walk meant to cover delayed agents "and those blocking their progress" ([JAIR](https://www.jair.org/index.php/jair/article/download/19324/27343)).

The quality gap to LaCAM* remains large. At 60 s on den520d, ADDRESS reports a sum of delays of **2,646 against 33,604 for LaCAM\***, and on the warehouse map **3,048 against 70,108** ([arXiv](https://arxiv.org/html/2408.02960v2)). DyMAB states that LaCAM's "higher costs confirm the advantage of anytime LNS methods at this scale" ([JAIR](https://www.jair.org/index.php/jair/article/download/19324/27343)). A careful SoCS 2025 re-benchmark found that **"rule-based heuristics serve as strong baselines"** while learned destroy methods show no clear advantage. It listed "targeting high-delayed agents" and "optimizing replan order" as open opportunities ([Tan et al.](https://arxiv.org/abs/2407.09451)), and the thesis addresses both.

| Contribution | Verdict | Closest prior work | What it needs |
|---|---|---|---|
| Free-cell path-length bound in SIPP | Close to folklore | MAPF-LNS2's space-time A* collapses to space A* after the last constraint ([code](https://github.com/Jiaoyang-Li/MAPF-LNS2/blob/master/src/SpaceTimeAStar.cpp)) | Present as a lemma plus a speed-up on *failing* PP calls; count expansions saved |
| Pruning, tie-breaking, heuristics in SIPP | Incremental engineering | SIPPS's `length_max` pruning and holding time ([code](https://github.com/Jiaoyang-Li/MAPF-LNS2/blob/master/src/SIPP.cpp)) | Ablations; do not oversell |
| Bounded-suboptimal SIPP as repair | Algorithm known; **use as SoC-improving LNS repair appears novel** | WSIPP/FocalSIPP ([ICAPS 2020](https://ojs.aaai.org/index.php/ICAPS/article/view/6674)); Rybecký et al., CTU ([SoCS 2021](https://imr.ciirc.cvut.cz/uploads/Papers-pdfs/Rybecky21socs.pdf)) | Frame it as "single-agent slack buys global SoC"; cite both |
| "Blocked" destroy operator | **Novel as a specific operator** | MAPF-LNS agent-based heuristic, ADDRESS, DyMAB H_sync | AUC comparison against ADDRESS/BALANCE; test it as a bandit arm |
| Suboptimal repair operators generally | Partly novel | LNS2 and LNS2+RL relax repair for feasibility only | Show the SoC gain, not just the speed |
| 2–3x faster re-implementation and visualizer | Engineering artifact | Unified LNS codebase ([mapf-lns-unified](https://github.com/ChristinaTan0704/mapf-lns-unified)) | Benchmark against official code under identical settings; open-source it |

The blocked operator has the clearest claim. MAPF-LNS, ADDRESS and DyMAB all capture blocking only *implicitly*, through random walks over path crossings. None of them builds neighborhoods from **causal chains of agents occupying another agent's goal**. Goal blocking is independently recognised as a key failure mode: an IJCAI 2026 SIPP-PP paper adds "goal reservation to prevent goal-blocking" ([IJCAI](https://ijcai.org/proceedings/2026/842)), and AOC-CBS discusses parked agents blocking later ones ([arXiv](https://arxiv.org/pdf/2608.08175)). The conceptual overlap with delay-based seeding is real, though. The safe framing is "a structural cause of delay that can be added as a bandit arm", not "a better seed rule".

For the bounded-suboptimal repair, cite Yakovlev et al. (ICAPS 2020) and Rybecký et al. (SoCS 2021, co-authored at CTU). Reviewers will know the single-agent algorithm, and the novelty lies in the effect on global cost, not in the search itself. The re-implementation speed-up is useful precisely because Tan et al. documented that baselines are often under-reported. The same paper means any "2–3x faster" claim must be measured against the official binary under identical time accounting ([SoCS 2025](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf)).

## Compare against four feasibility and four anytime baselines on the 25-scenario MovingAI protocol

**Feasibility / first solution.**

1. **In-house PP-RR** (plain SIPP-PP with restarts, that is, PSIPP without pushing) as the ablation.
2. **MAPF-LNS2** as the closest published competitor ([repo](https://github.com/Jiaoyang-Li/MAPF-LNS2)).
3. **LaCAM\*** (lacam2), single-threaded ([arXiv](https://arxiv.org/pdf/2305.03632)).
4. **lacam3** run with `--no-star --no-multi-thread` for the first solution, plus its defaults if thread counts are reported ([repo](https://github.com/Kei18/lacam3)).

Strong optional additions are **Goal-Branching LaCAM**, the newest dense-specific state of the art (code at github.com/Elysium8/gb-lacam2, license unchecked; [arXiv](https://arxiv.org/html/2611.07528)), and **LG-LaCAM** (MIT; [repo](https://github.com/allegorywrite/lg_lacam)). PIBT+ ([pibt2](https://github.com/Kei18/pibt2)) can serve as a fast but incomplete reference. **PBS** should be at least discussed and ideally run, because it is the systematic alternative to pushing ([repo](https://github.com/Jiaoyang-Li/PBS)). It is not a good scaling baseline: Tan et al. report that PP-style and PBS-style solvers fail to solve all scenarios within 10 s at scale ([SoCS 2025](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf)).

**Anytime quality.**

1. **MAPF-LNS** with the adaptive roulette, run from the MAPF-LNS2 binary.
2. **BALANCE** ([anytime-mapf](https://github.com/thomyphan/anytime-mapf)).
3. **ADDRESS**, whose repo also runs BALANCE through `--banditAlgo` ([repo](https://github.com/JimyZ13/ADDRESS)).
4. **lacam3** with anytime refinement on.
5. The user's own re-implementation with the original MAPF-LNS destroy set, as an ablation that separates implementation speed from algorithmic gain.

**DyMAB** should be added if its code is public, which the research did not confirm. Learned solvers (MAPF-GPT, PRIMAL3, LNS2+RL, DiffLNS) belong in related work only. They need POGEMA or GPU stacks, and independent evaluations find them no stronger than search ([PRIMAL3](https://arxiv.org/abs/2608.04905v1), [Tan et al.](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf)).

All of the classical baselines are C++, read MovingAI `.map`/`.scen` files, and run on Linux. The Okumura repos are MIT-licensed. The Li-lineage repos (MAPF-LNS2, EECBS, ADDRESS, BALANCE) are under a USC research-only license, which is fine for academic comparison ([license](https://github.com/Jiaoyang-Li/MAPF-LNS2/blob/master/license.txt)).

**Protocol.** Recent papers converge on the same setup:

- **Instances.** MovingAI maps with the **25 random scenarios** per map, taking the first k agents.
- **Time limits.** 30 s or 60 s, with 300 s for extended anytime runs.
- **Feasibility metrics.** Success rate and time to first solution.
- **Quality metrics.** Sum of delays, or SoC divided by the sum of shortest-path distances, plus **AUC** for anytime runs.

ADDRESS uses random-32-32-20, ost003d, den520d, warehouse-20-40-10-2-2 and Paris_1_256 at 60 s with up to 1,000 agents ([arXiv](https://arxiv.org/pdf/2408.02960)). Tan et al. add empty-32-32, cap the initial solution at 10 s, report final delay and AUC at 60 s and 300 s, and count only destroy and repair time ([SoCS 2025](https://www.cs.sfu.ca/~hangma/pub/socs25.pdf)). To claim dense wins, also adopt **GB-LaCAM's dense protocol**: maze-32-32-2/4 and room-32-32-4 at 60–90% occupancy, 30 s, and a single thread. Its published LaCAM* and LaCAM3 numbers then give a direct reference ([arXiv](https://arxiv.org/html/2611.07528)).

Two fairness traps are worth stating explicitly:

- **Threads.** lacam3 is multi-threaded by default, so pin it to one thread or report thread counts.
- **Cost conventions.** LaCAM's reported cost fields may follow a different convention. Recompute SoC from the output paths under the stay-at-target definition and validate them with your own vertex-and-swap conflict checker ([LG-LaCAM's flowtime definition](https://arxiv.org/html/2510.19072v2)).

Reviewers will also expect these ablations:

- the push limit (1, 5 or unlimited);
- the push cost;
- SIPP-first versus always using PSIPP;
- the "waits in previous cell" model versus a full replan;
- the ordering policy;
- an argument that recursive pushing terminates, related to the guarantees of Morag et al.

## Conclusion

The field has converged on PSIPP's idea from the other side. PIBT-derived methods have moved from single-step pushes toward multi-step, multi-agent displacement (EPIBT, MD-PIBT, PUSH), while PP has stayed mostly static at the low level. PSIPP sits where the two lines meet: PIBT-style displacement inside full-horizon SIPP-based PP. Its reversed priority direction and its role as an LNS initializer are what make it distinct. That window is narrow and closing. PUSH comes from Jiaoyang Li's group, and a one-shot follow-up is plausible, so speed of submission matters more than polishing secondary contributions.

For the thesis material, the strategic move is to fold the blocked operator and the bounded-suboptimal repair into the PSIPP paper's LNS stage, or into a separate SoCS-style operator paper evaluated as bandit arms alongside BALANCE and ADDRESS. On their own, the SIPP bound, the pruning changes and the faster code are unlikely to carry a paper. The single most persuasive result available is an anytime curve: PSIPP-initialized LNS beating MAPF-LNS2 and lacam3 on GB-LaCAM's dense maps, where LaCAM* fails on more than half the instances.
