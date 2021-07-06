Code, data, and analysis scripts for the paper:

* Mark J. Nelson (2021). [Estimates for the Branching Factors of Atari Games](https://www.kmjn.org/publications/AtariBranching_CoG21-abstract.html). In *Proceedings of the IEEE Conference on Games.*

This requires a patched version of the Arcade Learning Environment (ALE), available at https://github.com/NelsonAU/Arcade-Learning-Environment

Overview:

* [countstates.cpp](countstates.cpp) is the code to count distinct reachable states in ALE up to a threshold, using either breadth-first search (BFS) or iterative deepening (ID)
* Data up to 10,000 states on all 103 ALE games, using both BFS and ID, is in the directory [results10k](results10k), and aggregated in [validate10k.csv](validate10k.csv)
* Data up to 1,000,000 states on all games, using only BFS, is in the directory [results1m](results1m), and aggregated in [all1m.csv](all1m.csv)
* [analyze.R](analyze.R) was used to produce the main results table in the paper (Table II); the data for that table is in [all1m_results.csv](all1m_results.csv)
* [analyze_validation.R](analyze_validation.R) was used to produce the BFS vs. ID mismatches table (Table I); the data for that one is in [mismatches.csv](mismatches.csv)
* [rominfo.R](rominfo.R) gives the size of the minimal action set for each game, and whether it uses the paddle controller
