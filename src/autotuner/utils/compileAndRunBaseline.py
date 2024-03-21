#!/usr/bin/env python

import os
import sys

thisPath = os.path.dirname(os.path.abspath(__file__))

sys.path.append(thisPath)
import utils


def writeExecutionTimeFile(pathToFile, time):
  with open(str(pathToFile), 'w') as f:
    f.write(str(time))
    f.close()

  return


def compileAndRunBaseline():
  spaceFile = os.environ['autotunerSPACE_FILE']
  confFile = os.environ['INDEX_FILE']
  executionTimeFile = os.environ['autotunerEXECUTION_TIME']
  ranges, _ = utils.readSpaceFile(spaceFile)

  conf = {}
  for loopID in ranges:
    conf[loopID] = []
    for elem in ranges[loopID]:
      conf[loopID].append(0)

  # Compile
  compileRetCode = utils.myCompile(confFile, conf)
  if (compileRetCode != 0):
    sys.exit(1)

  # Run parallel optimized binary
  runRetCode = utils.myRun()
  if (runRetCode != 0):
    sys.exit(1)

  # Get execution time
  time = utils.readExecutionTimeFile(executionTimeFile)

  return time



if __name__ == '__main__':
  baselineTime = compileAndRunBaseline()
  baselineTimeFile = os.environ['autotunerBASELINE_TIME']
  writeExecutionTimeFile(baselineTimeFile, baselineTime)
