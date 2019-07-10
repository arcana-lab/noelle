CC=clang
CPP=clang++
OPT=opt

INSTALL_DIR=~/CAT
INSTALL_LIB=$(INSTALL_DIR)/lib

AA_BASE=-basicaa
AA_ADVANCED=$(AA_BASE) -globals-aa -cfl-steens-aa -tbaa -scev-aa -cfl-anders-aa
ANALYSES=$(AA_ADVANCED) -scalar-evolution -loops -loop-simplify -lcssa -domtree -parallelization 
LIBS=-lm

# Set the runtime flags
RUNTIME_CFLAGS="-DDEBUG"
#RUNTIME_CFLAGS="$(RUNTIME_CFLAGS) -DDSWP_STATS"
#RUNTIME_CFLAGS="$(RUNTIME_CFLAGS) -DRUNTIME_PRINT"

# Other parallelization options:
PARALLELIZATION_OPTIONS=-alloc-aa-verbose=1 -pdg-verbose=1 -noelle-verbose=2 -dswp-force -noelle-min-hot=0
OPT_LEVEL=-O3

INCLUDES=-I../../include/threadpool/include
THREADER=Parallelizer_utils
OPTIMIZED=parallelized

# Loader/dumper of PDGs
UNIQUE_IR_MARKER_PASS=-load $(INSTALL_LIB)/UniqueIRMarker.so 
ORACLEAA_ARGS=-input-args "${oracle_input_args}" -exe-script=$(INSTALL_DIR)/bin/run_via.sh
ORACLEAA_PASSES=$(UNIQUE_IR_MARKER_PASS) -load $(INSTALL_LIB)/OracleAA.so 
ORACLEAA=$(ORACLEAA_PASSES) $(ORACLEAA_ARGS)

TRANSFORMATIONS_BEFORE_PARALLELIZATION=$(AA_BASE) -mem2reg -simplifycfg -instcombine -tailcallelim -loop-simplify -lcssa -loop-unswitch -instcombine -loop-simplify -lcssa -indvars -loop-deletion -instcombine -indvars
PDGPASS=$(ORACLEAA) $(COMMPASSES) -load $(INSTALL_LIB)/AllocAA.so -load $(INSTALL_LIB)/TalkDown.so -load $(INSTALL_LIB)/PDGAnalysis.so 
PRINTPASSES=$(PDGPASS) -load $(INSTALL_LIB)/PDGPrinter.so
PARALLELIZATION_TECHNIQUES= -load $(INSTALL_LIB)/DSWP.so -load $(INSTALL_LIB)/DOALL.so -load $(INSTALL_LIB)/HELIX.so
PRE_OPTPASSES=-load $(INSTALL_LIB)/Architecture.so $(PDGPASS) -load $(INSTALL_LIB)/HotProfiler.so -load $(INSTALL_LIB)/Parallelization.so -load $(INSTALL_LIB)/DGSimplify.so
OPTPASSES=$(PDGPASS) -load $(INSTALL_LIB)Architecture.so -load $(INSTALL_LIB)/DataFlow.so -load $(INSTALL_LIB)/HotProfiler.so -load $(INSTALL_LIB)/Parallelization.so -load $(INSTALL_LIB)/Heuristics.so -load $(INSTALL_LIB)/ParallelizationTechnique.so $(PARALLELIZATION_TECHNIQUES) -load $(INSTALL_LIB)/Parallelizer.so 
PRINT_OUTPUT_FILE=test.txt

# Fetch the list of files to compile
CPP_FILES := $(wildcard *.cpp)
BC_FILES := $(notdir $(CPP_FILES:.cpp=.bc))

COMMPASSES=-load $(INSTALL_LIB)/CommutativeDependenceAnalysis.so
COMM_DEP_ANA=-CommutativeDependenceAnalysis

COMM_USE=$(COMM_DEP_ANA) -comm-dep=test.commd 
COMM=$(COMMPASSES) $(COMM_DEP_ANA) -comm-dep=test.commd 

all: baseline baseline_pdgdot.bc testseq $(OPTIMIZED)

%.bc: %.c
	$(CC) $(INCLUDES) -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
	$(OPT) $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $@ -o $@
	$(OPT) -dot-cfg -disable-output $@
	llvm-dis $@

%.bc: %.cpp
	$(CPP) $(INCLUDES) -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
	$(OPT) $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $@ -o $@
	$(OPT) -dot-cfg -disable-output $@
	llvm-dis $@

baseline: baseline.bc
	$(CPP) $(OPT_LEVEL) $^ -o $@


test_seq_comm.bc: test_seq.bc
	$(OPT) $(OPTPASSES) $(COMM_USE) -heuristics -parallelizer $(PARALLELIZATION_OPTIONS) --debug-pass=Structure -pdg-verbose=2 -noelle-verbose=2 $^ -o $@

test_seq_comm: test_seq_comm.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@

baseline_pdgdot.bc: test.bc
	$(OPT) $(PRINTPASSES) $(ANALYSIS) -PDGPrinter $^ -o $@

testseq: test_seq.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@
	
$(THREADER).bc: $(THREADER).cpp
	$(CPP) $(RUNTIME_CFLAGS) $(INCLUDES) -std=c++14 -emit-llvm $(OPT_LEVEL) -c $^ -o $@

$(OPTIMIZED): test_dswp.bc
	$(CPP) -std=c++14 -pthreads $(OPT_LEVEL) $^ $(LIBS) -o $@

test_pdgdot.bc: test_seq.bc
	$(OPT) $(PRINTPASSES) $(ANALYSES) -heuristics -PDGPrinter $^ -o $@

test_dswp_unoptimized.bc: test_seq.bc output.prof
	$(OPT) $(OPTPASSES) $(ANALYSES) -pgo-test-profile-file=output.prof -block-freq -pgo-instr-use -HotProfiler -heuristics -parallelizer $(PARALLELIZATION_OPTIONS) $< -o $@
	llvm-dis $@

test_dswp.bc: test_dswp_unoptimized.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@
	llvm-dis $@

ir_mark.bc: test.cpp
	$(CC) $(INCLUDES) -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
	@../../scripts/iterate_dgsimplify.sh "-inliner-verbose=1" "$(PRE_OPTPASSES)" "$@"
	$(OPT) $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $(UNIQUE_IR_MARKER_PASS) -UniqueIRID $@ -o $@
	$(OPT) -dot-cfg -disable-output $@
	llvm-dis $@

run_oracle_aa: $(THREADER).bc
	llvm-link test.in $^ -o test-in.bc
	$(OPT) $(PRINTPASSES) $(ANALYSIS) -use-oracle-aa -OracleAA test-in.bc -o $@

test.bc: test.cpp
	$(CC) $(INCLUDES) -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c $< -o $@
	$(OPT) $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $@ -o $@
	$(OPT) $(UNIQUE_IR_MARKER_PASS) -UniqueIRID  $@ -o $@
	$(OPT) -dot-cfg -disable-output $@

baseline.bc: test.bc
	$(CPP) $(OPT_LEVEL) -c -emit-llvm $^ -o $@
	llvm-dis $@

test_seq.bc: test.bc $(THREADER).bc
	llvm-link $^ -o $@
	@../../scripts/iterate_dgsimplify.sh "-inliner-verbose=1" "$(PRE_OPTPASSES)" "$@"
	$(OPT) $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $@ -o $@
	llvm-dis $@

output.prof: default.profraw
	llvm-profdata merge $^ -output=$@

default.profraw: test_seq_prof input.txt
	./$< `cat input.txt` 

test_seq_prof: test_seq_prof.bc
	$(CPP) -pthreads -fprofile-instr-generate -lstdc++ $^ $(LIBS) -o $@

test_seq_prof.bc: test_seq.bc
	@../../scripts/inject_hotprofiler.sh $^ $@

input.txt:
	@../../scripts/create_input.sh $@

show_passes: test_seq.bc
	$(OPT) $(OPTPASSES) $(ANALYSES) -heuristics -Parallelizer $(PARALLELIZATION_OPTIONS) $^ --debug-pass=Structure -disable-output

show_time: test_seq.bc
	$(OPT) $(OPTPASSES) $(ANALYSES) -heuristics -Parallelizer $(PARALLELIZATION_OPTIONS) $^ -time-passes -disable-output

print_transformations_before_parallelization:
	../../scripts/print.sh "$(PRINT_OUTPUT_FILE)" "$(TRANSFORMATIONS_BEFORE_PARALLELIZATION)" 

print_alias_analyses:
	../../scripts/print.sh "$(PRINT_OUTPUT_FILE)" "$(AA_ADVANCED)" 

print_analyses:
	../../scripts/print.sh "$(PRINT_OUTPUT_FILE)" "$(ANALYSES)" 

clean:
	rm -f *.bc *.dot *.jpg *.ll *.S *.s *.o *.txt baseline testseq $(OPTIMIZED) *.prof *.profraw *prof
    rm -f *.dep *.viaconf run_oracle_aa testseq_comm 

.PHONY: show_passes show_time
