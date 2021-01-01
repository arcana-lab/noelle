## Steps for constructing a FunctionTree

1. Split basic blocks if annotations change within
2. Construct root node and add loop container nodes as children to the tree

## What kind of information you can query from the Talkdown module
- `getAnnotationsForInst(Instruction *i)`

  Returns an std::unordered\_map that maps each loop that `i` is part of to a set annotations, e.g.\
  If `i` is inside an inner loop `L1` which is inside the outer loop `L0`, and\
    - there is a `"independent" = "1"` pragma around `L0`, and\
    - there is a `"independent" = "1"` pragma around `L1`, then

  `getAnnotationsForInst(i)` returns something like:\
      \{\
        'L0': \{ \{'independent', '1', 'L0'\} \},\
        'L1': \{ \{'independent', '1', 'L0'\}, \{'independent', '1', 'L1'\} \},\
      \}

## Notes

If annotation belongs to a loop header, then it should be attached to the loop container node (???)

Check in SemiLocalFunAA how LoopAA::alias() is called within a modref

Also look at KillFlow and CallSiteDepthCombinator to see how to get the topAA()

In PipelineStrategy, dump the number of sequential and parallel SCCs.

  Probably in PipelineStrategy::pStageWeightPrint()

Do we want to add BasicBlockContainerNode?

THE CODE IS SO UGLY NOW :( -- IGNORE THE UGLINESS PLS!!!

## Caveats

If a basic block is not contained within a loop, the annotations are ignored

A bunch of the strategies used to construct the tree work only because we are able to retrieve the loops
in a preorder. Not sure if this is a good way to go...
