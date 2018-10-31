: '
Goal: 
run the following as long as it outputs a file indicating to continue
opt $(TRANSFORMATIONS_BEFORE_PARALLELIZATION) $(PRE_OPTPASSES) -DGSimplify $@ -o $@
'

function expand_rel_path {
  local VAL=$1 ;
  # Was stripping quotes, no longer passes quotes
  # VAL=${VAL:0:${#VAL}-0} ;
  NEWVAL=${VAL//\~/$HOME} ;
  echo $NEWVAL ;
}

function runSimplify {
  local DEP_TRANS=$(expand_rel_path "$1") &> /dev/null ;
  local DEP_PASS=$(expand_rel_path "$2") &> /dev/null ;
  local FILE_NAME=$(expand_rel_path "$3") &> /dev/null ;
  local ITER=0 ;

  touch "dgsimplify_do_scc_call_inline.txt"
  touch "dgsimplify_continue.txt"
  while test -e "dgsimplify_continue.txt" ; do
    rm "dgsimplify_continue.txt"
    ((ITER++))
    printf "Running DGSimplify to inline calls within SCCs, iteration: $ITER\n"
    opt $DEP_TRANS $DEP_PASS "-DGSimplify" $FILE_NAME "-o" $FILE_NAME
  done

  ITER=0
  touch "dgsimplify_do_hoist.txt"
  touch "dgsimplify_continue.txt"
  while test -e "dgsimplify_continue.txt" ; do
    rm "dgsimplify_continue.txt"
    ((ITER++))
    printf "Running DGSimplify to hoist loops to main, iteration: $ITER\n"
    opt $DEP_TRANS $DEP_PASS "-DGSimplify" $FILE_NAME "-o" $FILE_NAME
  done
}

rm -f dgsimplify_**
runSimplify "$1" "$2" "$3"
rm -f dgsimplify_**
exit 0
