# Compute branching factor estimates from state count data
# Modified version of analyze.R specifically to look for BFS/ID disagreements
# on the valiation run.
# Mark Nelson, 2021

library(tidyverse)
library(polynom)

precision=2 # number of decimal places to write out

runname = "validate10k"

d <- read_csv(paste(runname,".csv", sep=""))

# see analyze.R for comments on this computation
est_bf <- function(s, d) {
    poly = as.function(polynomial(append(c(1-s), rep(1, d))))
    uniroot(poly, c(0.99, 18.01)) # for ALE, should be in range [1,18]
}
bfs <- d %>% group_by(game,method) %>% top_n(1) %>% mutate(bf=map2_dbl(states, depth, ~ est_bf(.x,.y)$root)) %>% ungroup()

bfs %>% select(game,method,bf) %>%
  pivot_wider(names_from=method,values_from=bf) %>% filter(bfs!=id) %>% arrange(desc(abs(bfs-id))) %>%
  mutate(bfs=sprintf("%.*f",precision,bfs),id=sprintf("%.*f",precision,id)) %>%
  select(Game=game,BFS=bfs,ID=id) %>%
  write_csv("mismatches.csv")
