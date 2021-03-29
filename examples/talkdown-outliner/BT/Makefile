#baseline_post_prof
#baseline_pre_prof
#default.profraw
#Makefile
#omp-critical.c
#omp-critical.ll
#omp-critical.ll_clean.bc
#omp-critical.ll_inlined.bc
#omp-critical.ll_norm.bc
#omp-critical.ll_outlined.bc
#omp-critical.ll_pdg_prof.bc
#omp-critical.ll_prof.bc
#omp-critical.ll_prof.ll
#omp-critical.ll_split.bc

all: src

src:
	.//noelle-mp bt.ll ;

clean:
	rm bt.ll_*
	rm baseline*
	rm default.profraw
	rm Parallelizer_utils.*
	rm -rf include
