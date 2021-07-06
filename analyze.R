# Compute branching factor estimates from state count data
# Mark Nelson, 2021

library(tidyverse)
library(polynom)

precision=2 # number of decimal places to write out

runname = "all1m"

d <- read_csv(paste(runname,".csv", sep=""))

# We have a cumulative count of (frame_num, total distinct states). To estimate
# branching factor, observe that, if average branching factor is b, the
# cumulative number of distinct game states observed at depth d should be:
#   states = \sum_{i=0}^d b^i
# We can rewrite this into a polynomial root-solving problem, solving for b in:
#   (1-states) + b + b^2 + ... b^n = 0
# using any of a number of computational methods. Here we use R's uniroot.
#
# Side notes:
# * I informally compared a few of the results to what the computer algebra
#   system Maxima produces using exact rational arithmetic, and they agree up
#   to 5+ decimal places. (I would just use Maxima, but R integrates into a
#   reproducible pipeline a lot more easily.)
# * Do not use R's polyroot(); besides being a bit of a pain to pull out the
#   real roots, it is numerically unstable on high-degree polynomials.
est_bf <- function(s, d) {
    poly = as.function(polynomial(append(c(1-s), rep(1, d))))
    uniroot(poly, c(0.99, 18.01)) # for ALE, should be in range [1,18]
}
bfs <- d %>% group_by(game) %>% top_n(1) %>%
  mutate(bf=map2_dbl(states, depth, ~ est_bf(.x,.y)$root))

# output/formatting stuff past here
rominfo <- read_csv("rominfo.csv")
bfs <- bfs %>% inner_join(rominfo,by="game")

bfs %>% arrange(desc(bf)) %>%
  mutate(bf=sprintf("%.*f",precision,bf)) %>% select(Game=game,Actions=num_actions,`Branching factor`=bf) %>%
  write_csv(paste(runname,"_results.csv",sep=""))
