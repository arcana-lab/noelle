
INSTALL_DIR=/local/scratch/josi2/noelle/install
INSTALL_LIB=$(INSTALL_DIR)/lib

CC=${LLVM_BIN}/clang++
AA_BASE=-basicaa
AA_ADVANCED=$(AA_BASE) -globals-aa -cfl-steens-aa -tbaa -scev-aa -cfl-anders-aa
ANALYSES=$(AA) -scalar-evolution -loops -loop-simplify -lcssa -domtree -parallelization -heuristics
LIBS=-lm

RUNTIME_CFLAGS="-DDSWP_STATS"
RUNTIME_CFLAGS=""
PARALLELIZATION_OPTIONS=-pdg-verbose=1 -dswp-verbose=1 -dswp-force -doall-chunk-size=32 -doall-cores-per=8 -dswp-cores-per=2
OPT_LEVEL=-O3

INCLUDES=-I../../include/threadpool/include
THREADER=Parallelizer_utils
OPTIMIZED=parallelized



TRANSFORMATIONS_BEFORE_PARALLELIZATION=$(AA_BASE) -mem2reg -simplifycfg -instcombine -tailcallelim -loop-simplify -lcssa -loop-unswitch -instcombine -loop-simplify -lcssa -indvars -loop-deletion -instcombine -indvars
ORACLEAA_PASS=-load $(INSTALL_LIB)/UniqueIRMarker.so -load $(INSTALL_LIB)/OracleAA.so  -input-args "${oracle_input_args}" -exe-script=$(INSTALL_DIR)/bin/run_via.sh -loops
PDGPASS=$(ORACLEAA_PASS) -load $(INSTALL_LIB)/PDGAnalysis.so
PRINTPASSES=$(PDGPASS) -load $(INSTALL_LIB)/PDGPrinter.so
PARALLELIZATION_TECHNIQUES= -load $(INSTALL_LIB)//DSWP.so -load $(INSTALL_LIB)/DOALL.so -load $(INSTALL_LIB)/HELIX.so
PRE_OPTPASSES=$(PDGPASS) -load $(INSTALL_LIB)/Parallelization.so -load $(INSTALL_LIB)/DGSimplify.so
OPTPASSES=$(PDGPASS) -load $(INSTALL_LIB)/Parallelization.so -load $(INSTALL_LIB)/Heuristics.so -load $(INSTALL_LIB)/ParallelizationTechnique.so $(PARALLELIZATION_TECHNIQUES) -load $(INSTALL_LIB)/Parallelizer.so 

all: baseline baseline_pdgdot.bc testseq $(OPTIMIZED)

baseline: baseline.bc
	@echo $(PATH)
	$(CC) $(OPT_LEVEL) $^ -o $@

baseline_pdgdot.bc: test.bc
	opt $(PRINTPASSES) $(ANALYSIS) -use-oracle-aa -PDGPrinter $^  -o $@

testseq: test_seq.bc
	$(CC) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@
	
$(THREADER).bc: $(THREADER).cpp
	$(CC) $(RUNTIME_CFLAGS) $(INCLUDES) -std=c++14 -emit-llvm $(OPT_LEVEL) -c $^ -o $@

$(OPTIMIZED): test_dswp.bc
	$(CC) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@

test_pdgdot.bc: test_seq.bc
	opt $(PRINTPASSES) $(ANALYSES) -PDGPrinter $^ -o $@

test_dswp.bc: test_seq.bc
	opt $(OPTPASSES) $(ANALYSES) -parallelizer $(PARALLELIZATION_OPTIONS) $^ -o $@
	$(CC) $(OPT_LEVEL) -c -emit-llvm $@ -o $@
	llvm-dis $@

baseline.bc: test.bc
	$(CC) $(OPT_LEVEL) -c -emit-llvm $^ -o $@
	llvm-dis $@

test.bc: test.cpp
	$(CC) $(INCLUDES) -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
#	opt $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $@ -o $@
	opt -dot-cfg -disable-output $@
	llvm-dis $@

test_seq.bc: test.bc $(THREADER).bc
	llvm-link $^ -o $@
	@../../scripts/iterate_dgsimplify.sh "-inliner-verbose=1" "$(PRE_OPTPASSES)" "$@"
	opt $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $@ -o $@
	llvm-dis $@

show_passes: test_seq.bc
	opt $(OPTPASSES) $(ANALYSES) -Parallelizer $(PARALLELIZATION_OPTIONS) $^ --debug-pass=Structure -disable-output

show_time: test_seq.bc
	opt $(OPTPASSES) $(ANALYSES) -Parallelizer $(PARALLELIZATION_OPTIONS) $^ -time-passes -disable-output

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o *.txt baseline testseq $(OPTIMIZED)

.PHONY: show_passes show_time
