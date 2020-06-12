#!/bin/bash -e

CC="clang++"
LIB_DIR=$PDG_INSTALL_DIR/lib
TEST_LIB_DIR=$PDG_INSTALL_DIR/test/lib
TRANSFORMATIONS_BEFORE_PARALLELIZATION="-basicaa -mem2reg -scalar-evolution -loops -loop-simplify -lcssa -domtree -postdomtree"

PROGRAM_INPUT_FOR_PROFILE="20 20 20"
TEST_PROFILE=output.prof

function loadAndRunNoellePasses {

  local PASSES=$1
  local INPUT=$2
  local OUTPUT=$3

  local BASICS="-load $LIB_DIR/Architecture.so -load $LIB_DIR/BasicUtilities.so -load $LIB_DIR/DataFlow.so \
    -load $LIB_DIR/Loops.so"
  local ANALYSES="-globals-aa -cfl-steens-aa -tbaa -scev-aa -cfl-anders-aa"
  local WPAPASS="-load $LIB_DIR/libSvf.so -load $LIB_DIR/libCudd.so \
    -veto -nander -hander -sander -sfrander -wander -ander -fspta -lander -hlander -stat=false"
  local PDGPASS="-load $LIB_DIR/AllocAA.so -load $LIB_DIR/TalkDown.so -load $LIB_DIR/PDGAnalysis.so"
  local ENABLERS="-load $LIB_DIR/LoopDistribution.so"
  local PARALLELIZATION_TECHNIQUES="-load $LIB_DIR/DSWP.so -load $LIB_DIR/DOALL.so -load $LIB_DIR/HELIX.so"

  local OPTPASSES="$WPAPASS $PDGPASS $BASICS \
    -load $LIB_DIR/HotProfiler.so \
    -load $LIB_DIR/Parallelization.so \
    -load $LIB_DIR/Heuristics.so -load $LIB_DIR/ParallelizationTechnique.so \
    $ENABLERS $PARALLELIZATION_TECHNIQUES -load $LIB_DIR/Parallelizer.so"

  #local RUNNERS="-parallelization -block-freq -pgo-test-profile-file=$TEST_PROFILE -pgo-instr-use -HotProfiler -heuristics $PASSES"
  local RUNNERS="-parallelization -block-freq -heuristics $PASSES"
  local CMD_TO_EXECUTE="opt $OPTPASSES $ANALYSES $RUNNERS $INPUT -o $OUTPUT"
  eval $CMD_TO_EXECUTE
}

function runTest {
  if ! test -d $2 ; then
    return ;
  fi
  if ! test -f $2/test.cpp ; then
    return ;
  fi

  local TEST_SO="$1.so"
  cd $2 ;

  echo -e "$1: Testing $2 " ;

  rm -f *.bc *.ll *.s *.o ;

  ${CC} -std=c++14 -emit-llvm -O0 -Xclang -disable-O0-optnone -c test.cpp -o test_pre.bc

  # noelle-prof-coverage test_pre.bc test_pre_prof
  # ./test_pre_prof "$PROGRAM_INPUT_FOR_PROFILE"
  # llvm-profdata merge default.profraw -output=$TEST_PROFILE

  # noelle-meta-prof-embed test_pre.bc -o test_prof.bc
  # ${CC} -O0 -fprofile-instr-generate test_prof.bc -o test_prof

  opt ${TRANSFORMATIONS_BEFORE_PARALLELIZATION} test_pre.bc -o test.bc &> /dev/null
  llvm-dis test.bc -o test.ll

  local UNIT_TEST_PASS="-load $TEST_LIB_DIR/UnitTestHelpers.so -load $TEST_LIB_DIR/$TEST_SO -UnitTester"
  loadAndRunNoellePasses "$UNIT_TEST_PASS" test.bc tested.bc &> compiler_output.txt
  llvm-dis tested.bc -o tested.ll

  if test -f "test_output.txt" ; then
    # cat test_output.txt
    :
  else
    echo "Test suite did not output results to test_output.txt"
  fi

  cd ../ ;
}

function checkRunSuite {
  local TestSuiteName=$1
  local UnitTestName=$2

  if ! test -d $TestSuiteName ; then
    return ;
  fi
  if ! test -d $TestSuiteName/suite ; then
    return ;
  fi

  cd $TestSuiteName/suite/ ;

  if ! test -z "$UnitTestName" ; then
    runTest $TestSuiteName $UnitTestName
    grep "Summary" $UnitTestName/test_output.txt
  else
    for i in `ls`; do
      runTest $TestSuiteName $i ;
      grep "Summary" $i/test_output.txt
    done
  fi

  echo -e "Finished running suite: $TestSuiteName" ;

  cd ../../ ;
}

function trimTrailingSlash {
  sed 's/\/$//' <<< $1
}

SUITE=$(trimTrailingSlash $1)
UNIT=$2

checkRunSuite $SUITE $UNIT ;
