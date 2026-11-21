# PSIPP (Push-SIPP inside Prioritized Planning): novelty and closest related work (as of Nov 2026)

Idea under evaluation: one-shot MAPF. Prioritized Planning (PP) with random restarts; ordering is longest-first on the first attempt and random afterwards. Each agent is planned by plain SIPP against already-planned agents. If SIPP fails, PSIPP runs: when the search enters a cell occupied by an already-planned (higher-priority) agent X during X's interval, it may "push" X, meaning it assumes X waits in its previous cell. This generates extra nodes, with at most about 5 pushes and a configurable push cost. Pushed agents are then replanned recursively (SIPP first, then PSIPP). A cycle or failure restarts PP with a new ordering. PSIPP provides the initial solution for an MAPF-LNS2-style anytime solver.

Search budget note: about 15 web searches and fetches (Nov 21, 2026). Each claim is marked as either verified in this session (a source was fetched or a search snippet was read) or "[from prior knowledge, link not re-verified]".

## Q1. Has anyone published PP/SIPP where the low-level planner can push or displace already-planned agents and then replan them?

### Takeaway
The closest prior work is **PUSH** (Sanjay & Li, arXiv, Aug 2026). It is a windowed space-time A* in which a path may displace one other agent, and the displaced agent is then replanned recursively. That is the same core mechanism as PSIPP. PUSH differs in four ways: it is lifelong, windowed, has no SIPP, and only lets higher-priority agents displace lower-priority ones, which is the reverse of PSIPP. The PIBT family (PIBT, winPIBT, EPIBT, MD-PIBT) contains the same "displace and recursively replan" idea at the configuration or short-horizon level. I found no paper that does **one-shot, full-horizon PP with a SIPP low-level search that pushes already-planned higher-priority agents and recursively replans them**. Within this search budget, PSIPP looks novel as a specific combination. It is not novel as a general concept.

### Cited Findings
**Closest: PUSH (2026)**
- V. Sanjay, J. Li. "Scalable Long-Horizon Planning with Staggered Updates for Lifelong MAPF." arXiv:2608.06702, submitted 7 Aug 2026. One version, no venue listed. [arXiv](https://arxiv.org/abs/2608.06702), [HTML](https://arxiv.org/html/2608.06702v1)
  - PUSH ("Path Updates over Staggered Horizons") plans only a subset of agents per timestep using staggered windows. It produces RHCR-style windowed paths and adds "EPIBT-inspired priority inheritance, backtracking, and anytime improvements". It scales to about 10k agents. [arXiv abstract](https://arxiv.org/abs/2608.06702)
  - The low level is a modified windowed space-time A*, **not SIPP**. A search state records the displaced agent. Repeated collisions with that same agent are allowed, but collisions with several different agents are not, so each path displaces **at most one** agent. [HTML](https://arxiv.org/html/2608.06702v1)
  - Agents in the protected set ℋ (higher-priority agents and agents executing tasks) cannot be displaced. If the path collides with a displaced agent r_d, PushPlan is called recursively on r_d with ℋ ∪ {r_i}. Recursion depth is bounded because each pushing agent joins ℋ. A visit counter limits how often each agent can be revisited (bound M). [HTML](https://arxiv.org/html/2608.06702v1)
  - When planning fails, PUSH restores the old path and returns false, which makes the preceding agent replan. It records the failed collision (r_d, t_d) as a constraint and searches again instead of enumerating alternative paths. [HTML](https://arxiv.org/html/2608.06702v1)
  - It uses W=10 and K=5. Priorities are "close goal" or "far goal", and persist as in PIBT. The anytime part is an LNS-like loop: replan a random agent with PushPlan and accept if Σh(endpoint, goal) decreases. [HTML](https://arxiv.org/html/2608.06702v1)
  - **How it differs from PSIPP:** (a) It is lifelong and windowed (10 steps), not one-shot with full-horizon paths to goal and sum of costs. (b) It uses space-time A*, not SIPP intervals. (c) A higher-priority agent displaces a **lower**-priority one, which is PIBT semantics. PSIPP lets the agent being planned now push **already-planned, higher-priority** agents, which turns PP's direction around. (d) It allows at most one displaced agent per path, while PSIPP allows up to about 5 pushes. (e) A failure backtracks to the parent in the chain, while PSIPP restarts the whole PP attempt with a new random order. (f) Pushing in PUSH means a full replan of the displaced agent's window. PSIPP pushes with a specific "X waits in its previous cell" model inside the search, then replans X.
  - PUSH cites the following as its pushing lineage: PIBT (Okumura et al., AIJ 2022), EPIBT (Yukhnevich & Andreychuk, AAAI 2026), MD-PIBT (Jiang et al., IROS 2026), Token Passing (Ma et al. 2017), and LaCAM. Per this fetch, it cites no earlier "prioritized planning with pushing" paper. [HTML](https://arxiv.org/html/2608.06702v1)

**The PIBT family (displacement plus recursive replanning at the configuration level)**
- K. Okumura, M. Machida, X. Défago, Y. Tamura. "Priority Inheritance with Backtracking for Iterative Multi-agent Path Finding." IJCAI 2019. Extended version in Artificial Intelligence, 2022. [IJCAI'19](https://ijcai.org/proceedings/2019/76), [arXiv](https://arxiv.org/html/1901.11282). When a lower-priority agent blocks a higher-priority one, it temporarily inherits the higher priority and is pushed. Backtracking prevents deadlock. [search summary](https://arxiv.org/html/1901.11282)
  - **Differs:** It plans one timestep at a time with no full-horizon paths, so it has no PP or SIPP. Pushing goes from higher to lower priority among agents not yet planned at that step.
- K. Okumura et al. "winPIBT: Extended Prioritized Algorithm for Iterative Multi-agent Path Finding." arXiv:1905.10149, 2019. [arXiv](https://arxiv.org/pdf/1905.10149). It says priority inheritance resembles a "push" operation but has no "swap", and that some instances fail under every fixed static priority order. [search snippet](https://arxiv.org/pdf/1905.10149)
  - **Differs:** It plans paths over a window rather than full horizon, and it is lifelong.
- E. Yukhnevich, A. Andreychuk. "Enhancing PIBT via Multi-Action Operations" (EPIBT). AAAI 2026, vol. 40(35). arXiv:2511.09193 (Nov 2025). [AAAI](https://ojs.aaai.org/index.php/AAAI/article/view/40233), [arXiv](https://arxiv.org/abs/2511.09193). It combines rotations, moves, and waits into multi-action operations, allows agents to be revisited within one step, and is state of the art in LMAPF-T together with guidance and LNS. [search summary](https://arxiv.org/abs/2511.09193) PUSH describes it as pushing at most one lower-priority agent per path. [PUSH HTML](https://arxiv.org/html/2608.06702v1)
  - **Differs:** It is lifelong, uses short multi-action horizons, and enumerates precomputed W-step paths instead of running a SIPP search.
- Z. Jiang, Y. Zhang, R. Veerapaneni, J. Li. "Planning over MAPF Agent Dependencies via Multi-Dependency PIBT" (MD-PIBT). arXiv:2603.23405 (Mar 2026). mapf.info lists it as IROS 2026. [arXiv](https://arxiv.org/abs/2603.23405), [MSR page](https://www.microsoft.com/en-us/research/publication/planning-over-mapf-agent-dependencies-via-multi-dependency-pibt/), [mapf.info](https://mapf.info/index.php/Main/JLi26m). It searches over agent dependencies instead of space-time constraints. It reproduces PIBT and EPIBT as special cases and supports multi-step paths that collide with **more than one** agent. Code: github.com/lunjohnzhang/MD-PIBT. [search summary](https://arxiv.org/abs/2603.23405)
  - **Differs:** It is still PIBT-style and short-horizon. Its "push multiple agents" option is the nearest analogue to PSIPP's "up to 5 pushes". **A reviewer could cite this.**
- H. Nagai, K. Okumura. "From Gridworlds to Warehouses: Adapting Lightweight One-shot Multi-Agent Pathfinding for AGVs." IJCAI 2026, pp. 306–313. [IJCAI](https://ijcai.org/proceedings/2026/35). It is one-shot and AGV-specific, using multi-step PIBT with "stop paths". When a candidate path conflicts with an unplanned agent's stop path, that agent inherits priority and is recursively replanned, with backtracking on failure. Its PP baseline is space-time A* with a reservation table and has no pushing. PP and LNS2 struggle with many agents, while LaCAM+PIBT scales better at a higher cost. [IJCAI page](https://ijcai.org/proceedings/2026/35)
  - **Differs:** Recursive replanning targets agents that are **not yet planned**, not committed higher-priority paths, and there is no SIPP.

**Classical "push" in MAPF (rule-based, complete for some graph classes)** [from prior knowledge, links not re-verified]
- R. Luna, K. Bekris. "Push and Swap: Fast Cooperative Path-Finding with Completeness Guarantees." IJCAI 2011. https://www.ijcai.org/Proceedings/11/Papers/059.pdf
- B. de Wilde, A. ter Mors, C. Witteveen. "Push and Rotate: a Complete Multi-agent Pathfinding Algorithm." JAIR 51, 2014. https://www.jair.org/index.php/jair/article/view/10882
  - **Differ:** These are macro-operator, sequential-move algorithms with no PP or SIPP and poor solution quality. "Push" there means moving blocking agents aside, which is conceptually related.
- DisCoF combines decoupled prioritized planning with a fallback to coupled push-and-swap when conflicts occur. [search summary citing it](https://arxiv.org/pdf/1812.06356) (authors and venue not verified in this session).

**Searching over priorities (an alternative to pushing)**
- H. Ma, D. Harabor, P. J. Stuckey, J. Li, S. Koenig. "Searching with Consistent Prioritization for Multi-Agent Path Finding" (PBS). AAAI 2019. [arXiv](https://arxiv.org/pdf/1812.06356), [AAAI](https://ojs.aaai.org/index.php/AAAI/article/download/4758/4636). It notes that PP depends on "rule-of-thumb heuristics and random assignment" for a fixed total order, and that PP is incomplete in general. PBS searches lazily over partial priority orders, and when a conflict occurs the lower-priority agent and its dependants are replanned. [arXiv](https://arxiv.org/pdf/1812.06356)
  - **Differs:** On a conflict, PBS branches on which of the two agents gets priority (a DFS over orders). PSIPP keeps the order but lets the lower-priority agent change a higher-priority agent's plan locally. **Reviewers will almost certainly ask for a comparison against PBS.** PSIPP can be framed as a greedy, non-branching, single-path cousin of PBS.
- J. Morag, Y. Zhang, D. Koyfman, Z. Chen, A. Felner, D. Harabor, R. Stern. "Prioritised Planning with Guarantees." SoCS 2024, pp. 82–90. [SoCS](https://ojs.aaai.org/index.php/SOCS/article/view/31545). It introduces PCS, which it calls the first prioritised algorithm with completeness and optimality guarantees, and PPR*, which uses randomisation to sample the space. The extended version is "Prioritised Planning: Completeness, Optimality, and Complexity," JAIR 2025. [JAIR PDF](https://www.jair.org/index.php/jair/article/download/19358/27244) [search summary](https://ojs.aaai.org/index.php/SOCS/article/view/31545)
  - **Differs:** It searches over priority orders and paths with guarantees and does not push. PPR* (PP with random restarts) is the formal baseline for PSIPP's restart loop.

### Inferences
- The core concept "the low-level search may displace another agent, which is then recursively replanned" is established: PIBT since 2019, EPIBT and MD-PIBT in 2025–26, and PUSH in 2026 with an explicit windowed space-time A* that records displaced agents in the search state. PSIPP cannot claim the concept. It can claim **a SIPP-interval version for one-shot, full-horizon PP that pushes committed higher-priority agents, with a "pushed agent waits in its previous cell" model, used as a SIPP fallback inside PP with restarts and as an LNS initializer.**
- PUSH (Aug 2026, Jiaoyang Li's group) is the most dangerous overlap. It is very recent, from a central MAPF group, and it uses the same name. Avoid calling the method "PUSH". Cite it and explain the differences (one-shot, SIPP, reversed priority direction, multiple pushes, restart-on-failure).

### Gaps
- I could not check whether PUSH has a later version or venue acceptance, for example ICRA/RA-L 2027 or AAAI 2027.
- I did not fully check the MAPF-among-movable-obstacles PP paper (Hu, Zhao, Ren, arXiv:2509.26050, 2025, [arXiv](https://arxiv.org/abs/2509.26050)) for how its PP handles pushing. It pushes passive obstacles, not planned agents, and is small scale (≤20 agents).
- "Elevating priorities for an efficient and complete lifelong multi-AGV pathfinding on roadmaps" (Robotics and Autonomous Systems, 2025, [ScienceDirect](https://www.sciencedirect.com/science/article/abs/pii/S0921889025003926)) uses SIPP with priority elevation. It returned 403, so I could not verify its mechanism. It may be a SIPP-based priority-change method. **Worth checking manually.**
- I did not find "SIPP with pushing" papers in continuous or any-angle settings (the AA-SIPP-m line by Yakovlev and Andreychuk), but not finding them is weak evidence given the budget.

## Q2. 2023–2026 papers on improving PP for dense instances, and PP as an LNS initializer

### Takeaway
Recent PP work moves in three directions: (1) guarantees and theory (Morag et al. SoCS'24/JAIR'25), (2) learned priorities (Zhang et al. SoCS'22; attention-based AAMAS'24; RL-RH-PP 2026), and (3) better initial solutions for LNS2 (LNS2+RL AAAI'25; discrete diffusion initializer 2026). Recent dense benchmarks point the same way: PP and LNS2 struggle at high density, while PIBT and LaCAM scale. This is exactly the gap PSIPP targets.

### Cited Findings
- J. Li, Z. Chen, D. Harabor, P. J. Stuckey, S. Koenig. "MAPF-LNS2: Fast Repairing for Multi-Agent Path Finding via Large Neighborhood Search." AAAI 2022. [PDF](https://idm-lab.org/bib/abstracts/papers/aaai22a.pdf). This is the base framework. It uses PP with SIPPS (soft-constraint SIPP) to build an initial plan that may contain collisions and then repairs the collisions with LNS. [from prior knowledge plus the PDF link from search]
  - **Differs:** LNS2 tolerates collisions and repairs them. PSIPP aims for a collision-free PP solution by pushing. **Reviewers will ask why you don't just use PP+SIPPS followed by LNS2 repair.** A head-to-head on the time to reach a first feasible solution is essential.
- Y. Wang, T. Duhan, J. Li, G. Sartoretti. "LNS2+RL: Combining Multi-Agent Reinforcement Learning with Large Neighborhood Search in Multi-Agent Path Finding." AAAI 2025. [AAAI](https://ojs.aaai.org/index.php/AAAI/article/view/34501/36656), [arXiv](https://arxiv.org/html/2405.17794). The paper notes that SIPPS can return paths with far more soft collisions than necessary, because its heuristic ignores soft collisions while waiting. LNS2+RL reaches 100% SR where LaCAM gets 0% on maps with complex obstacles. On random-small maps LaCAM has a higher SR, but is about 7x slower than PP+SIPPS. [search summary](https://arxiv.org/html/2405.17794) (Authors are from prior knowledge, not re-verified.)
- "Discrete Diffusion for Complex and Congested Multi-Agent Path Finding with Sparse Social Attention." arXiv:2605.13296 (May 2026). [arXiv](https://arxiv.org/html/2605.13296). It argues that initialization quality is key for LNS2, and that a diffusion initializer gives more repairable plans than repeated PP initializations under the same multi-start budget. [search summary] (Authors not captured.)
  - **Relevance:** It directly competes on "a better LNS2 initializer in congested instances". It should be cited and, ideally, compared against.
- "Enhancing Lifelong Multi-Agent Path-finding by Using Artificial Potential Fields." arXiv:2505.22753 (2025). [arXiv](https://arxiv.org/pdf/2505.22753). It says PrP "might not be very effective in very dense environments due to possible deadlocks", and that LaCAM variants solved the majority of problems. [search summary]
- MAPF-HD: "Multi-Agent Path Finding in High-Density Environments." arXiv:2509.06374 (2025). [arXiv](https://arxiv.org/html/2509.06374). Its PHANS heuristic uses PP with A* and gives priority to agents with longer start-goal distances, so longest-first ordering appears in dense settings. [search summary]
- Nagai & Okumura, IJCAI 2026 (above): PP and LNS2 struggle with many agents, and LaCAM+PIBT scales. [IJCAI](https://ijcai.org/proceedings/2026/35)
- LaCAM / LaCAM* / "Engineering LaCAM*" (K. Okumura, AAAI 2023, IJCAI 2023, AAMAS 2024) is the state of the art for dense one-shot MAPF and solves thousands of agents in seconds with eventual optimality. [from prior knowledge, links not re-verified: https://arxiv.org/abs/2211.13432 (LaCAM), https://arxiv.org/abs/2305.03632 (LaCAM*), https://arxiv.org/abs/2308.04292 (Engineering LaCAM*)]
  - **Relevance:** Reviewers will ask how PSIPP compares with LaCAM* on success rate and sum of costs, and with LaCAM-initialized LNS. Since PSIPP's selling point is "same SoC as PP but solves far more", the LaCAM comparison should show that PSIPP has a better SoC than LaCAM's first solution.
- Other 2025–26 PP preprints that are relevant to density but are lifelong or domain-specific:
  - CPLP, "Prioritized Planning for Continuous-time Lifelong MAPF," arXiv:2503.13175 (up to 800 agents). [arXiv](https://arxiv.org/abs/2503.13175)
  - Complete prioritized planner for ordered storage at 100% load, arXiv:2608.07734. [arXiv](https://arxiv.org/html/2608.07734)
  - Hybrid prioritized framework decoupling geometric planning and execution, arXiv:2603.26684. [arXiv](https://arxiv.org/html/2603.26684)
  - Dual-Ordering anytime PP for conveyor routing, arXiv:2605.13035. [arXiv](https://arxiv.org/pdf/2605.13035)

  These come from search summaries only.

### Inferences
- The dense one-shot regime has shifted toward PIBT and LaCAM. A PP-based method must justify itself on solution quality (SoC) and success rate together. The claim "solves far more than SIPP-PP with the same SoC" is useful, but needs LaCAM* and LNS2 (PP+SIPPS with repair) baselines to convince reviewers.

### Gaps
- I did not find a 2023–26 one-shot paper that explicitly improves PP's success rate in dense instances by changing the low-level planner (rather than the ordering). This supports novelty, but the search was limited.
- I did not verify a survey titled "Prioritized planning algorithms for MAPF: a survey". None surfaced. Čáp et al.'s T-ASE 2015 "Prioritized Planning Algorithms for Trajectory Coordination of Multiple Mobile Robots" (revised and asynchronous decentralized PP) is the classic reference [from prior knowledge, https://arxiv.org/abs/1409.2399].

## Q3. Agent-ordering heuristics for PP: state of the art?

### Takeaway
Fixed heuristics (distance-based, longest-first or shortest-first) combined with random restarts remain the standard practical baseline. Learned orderings (Zhang et al. SoCS'22; attention-based AAMAS'24; RL-RH-PP 2026) give only moderate gains in one-shot MAPF. PSIPP's "longest-first, then random restarts" scheme is reasonable but not a contribution in itself.

### Cited Findings
- S. Zhang, J. Li, T. Huang, S. Koenig, B. Dilkina. "Learning a Priority Ordering for Prioritized Planning in Multi-Agent Path Finding." SoCS 2022, 15(1):208–216, doi:10.1609/socs.v15i1.21769. [SoCS](https://ojs.aaai.org/index.php/SOCS/article/view/21769). It proposes ML-T and ML-P with stochastic ranking and random restarts. It beats existing PP on small maps in most cases and stays competitive on large maps. [search summary]
- The 2026 RL-RH-PP paper reports that the 2022 approach achieved "moderate improvements over random priority ordering" but struggled to consistently beat strong heuristics such as query-distance. It also finds that RH-PP with randomly sampled total orders "performs surprisingly well" in lifelong MAPF. [arXiv 2603.23838](https://arxiv.org/html/2603.23838)
  - "Learning-guided Prioritized Planning for Lifelong Multi-Agent Path Finding in Warehouse Automation," arXiv:2603.23838 (2026). Authors per search: Zheng, Ma, Araki, Chen, Wu, with a JAIR 2026 attribution (not independently verified). [arXiv](https://arxiv.org/pdf/2603.23838)
- "Attention-based Priority Learning for Limited Time Multi-Agent Path Finding." AAMAS 2024 (MARMot Lab, NUS). [PDF](https://marmotlab.org/publications/54-AAMAS2024-LearnedPP.pdf), [AAMAS proceedings](https://ifaamas.org/Proceedings/aamas2024/pdfs/p1993.pdf)
- PBS (AAAI 2019) states that PP algorithms rely on "rule-of-thumb heuristics and random assignment". [arXiv](https://arxiv.org/pdf/1812.06356)
- A learned initial priority ordering for PBS in lifelong MAPF is the subject of an SFU thesis, "Learning to Prioritize: Accelerating Priority-Based Search for Lifelong…" (2026). [PDF](https://summit.sfu.ca/_flysystem/fedora/2026-01/etd24107.pdf)
- MAPF-HD (2025) prioritizes agents with longer start-goal distances. [arXiv](https://arxiv.org/html/2509.06374)
- [From prior knowledge, not re-verified] The classic ordering references are:
  - Berg & Overmars (IROS 2005), the distance-based "longest first" heuristic.
  - Bennewitz, Burgard, Thrun (RAS 2002), hill-climbing / randomized search over priority schemes.
  - Andreychuk & Yakovlev, "Two Techniques That Enhance the Performance of Multi-Robot Prioritized Path Planning" (AAMAS 2018 extended abstract): start/goal-conflict-based ordering plus restarts.
  - Čáp et al. (T-ASE 2015).
  - Hvězda, Kulich, Přeučil, "Context-Aware Route Planning for Automated Warehouses" (ITSC 2018).

### Inferences
- Reviewers will expect at least random and distance-based orderings, ideally the "start/goal conflict" ordering, and possibly PBS as the systematic alternative. Learned orderings are optional for a one-shot paper.

### Gaps
- I found no published head-to-head of longest-first vs. shortest-first vs. random restarts for one-shot SIPP-PP at high density. The user's own experiments may be the first clean data point.

## Q4/Q5. Any very close work at AAAI/IJCAI/ICAPS/AAMAS/SoCS/ICRA/IROS/RA-L/ECAI 2023–2026? Verdict and expected reviewer requests

### Takeaway
Nothing identical was found at those venues. The strongest "closest" works are:
1. PUSH (arXiv Aug 2026, not yet at a venue), which shares the mechanism.
2. MD-PIBT (IROS 2026) and EPIBT (AAAI 2026), which share the displace-and-replan concept.
3. PBS (AAAI 2019) and PCS/PPR* (SoCS 2024 / JAIR 2025), which repair priorities instead of pushing.
4. MAPF-LNS2 (AAAI 2022) and LNS2+RL (AAAI 2025), the same use-case of PP initialization for LNS.

PSIPP looks publishable as an incremental but practical contribution. SoCS, an ICAPS short or long paper, AAMAS, or a robotics venue (ICRA/IROS/RA-L) are the realistic targets. AAAI/IJCAI would need stronger results against LaCAM* and LNS2.

### Cited Findings
- See Q1–Q3 for the venue and year of each closest work: PUSH [arXiv 2608.06702](https://arxiv.org/abs/2608.06702); MD-PIBT [arXiv 2603.23405](https://arxiv.org/abs/2603.23405); EPIBT [AAAI 2026](https://ojs.aaai.org/index.php/AAAI/article/view/40233); PBS [AAAI 2019](https://arxiv.org/pdf/1812.06356); PCS/PPR* [SoCS 2024](https://ojs.aaai.org/index.php/SOCS/article/view/31545); MAPF-LNS2 [AAAI 2022](https://idm-lab.org/bib/abstracts/papers/aaai22a.pdf); LNS2+RL [AAAI 2025](https://ojs.aaai.org/index.php/AAAI/article/view/34501/36656); Nagai & Okumura [IJCAI 2026](https://ijcai.org/proceedings/2026/35).

### Inferences
**Verdict:** Novelty is likely sufficient for SoCS, ICAPS, AAMAS, or ICRA/IROS/RA-L. The method is a clean, simple modification (pushing inside SIPP, used only when SIPP fails, with restarts), and it gives large success-rate gains on standard MovingAI benchmarks. Positioning should be "bringing PIBT-style displacement into full-horizon SIPP-based PP for one-shot MAPF". Present the reversed direction (a lower-priority agent pushes committed higher-priority agents) as the distinguishing feature against PUSH and EPIBT.

**What reviewers are likely to ask (things to cite or compare):**
1. **Relation to PUSH (Sanjay & Li 2026), EPIBT, and MD-PIBT.** Explain why SIPP-based pushing differs, and why a one-shot full horizon matters. Rename the method if needed to avoid the name clash with PUSH.
2. **Baselines:**
   - PP+SIPP with random restarts (PPR*-like).
   - **PBS**.
   - **MAPF-LNS2** (PP+SIPPS initial solution with collision repair): time to first feasible solution and success rate.
   - **LaCAM*** and Engineering LaCAM* (success rate, SoC, runtime).
   - Ideally, LaCAM-initialized LNS.
3. **Theory:** Is PSIPP complete in any class (well-formed instances)? Does recursive pushing terminate? Cycle detection is handled by restart, but a bound or argument is expected. Relate this to the PP guarantees of Morag et al. 2024/2025.
4. **Ablations:**
   - Push limit (1 vs. 5 vs. unlimited); PUSH and EPIBT use 1 and MD-PIBT allows several.
   - Push cost.
   - SIPP-first vs. PSIPP-always.
   - Ordering (longest-first, random, start/goal-conflict ordering).
   - The "pushed agent waits in previous cell" model vs. a full replan.
5. **Downstream LNS:** Show that a better initial solution improves anytime SoC curves compared with LNS2's own initializer and with the diffusion initializer (arXiv 2605.13296), and that the gain is not just from success rate.
6. **Classical push/swap lineage:** Cite Push and Swap (IJCAI 2011) and Push and Rotate (JAIR 2014) for the terminology.

### Gaps
- My venue sweep was by keyword search, not a full read of AAAI-26, IJCAI-26, ICAPS-26, SoCS-26, AAMAS-26, ICRA-26, IROS-26, or ECAI-26 proceedings. A manual DBLP check for "push" plus "prioritized" in 2026 venues, and of the mapf.info 2026 list, is advised before submission.
- I could not confirm whether PUSH, or a one-shot follow-up of it, has been accepted somewhere. Monitor arXiv for v2.
