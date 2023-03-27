#!/usr/bin/env python

import os
import sys
import json
import traceback

thisPath = os.path.dirname(os.path.abspath(__file__))

sys.path.append(thisPath + "/../utils")
import utils

class Filter:
  spaceFile = None
  confFile = None
  executionTimeFile = None
  baselineTimeFile = None
  baselineTime = None
  ranges = None

  def getArgs(self):
    self.spaceFile = os.environ['autotunerSPACE_FILE']
    self.confFile = os.environ['INDEX_FILE']
    self.executionTimeFile = os.environ['autotunerEXECUTION_TIME']

    self.ranges, _ = utils.readSpaceFile(self.spaceFile)

    self.baselineTimeFile = os.environ['autotunerBASELINE_TIME']
    self.baselineTime = utils.readExecutionTimeFile(self.baselineTimeFile)

    return


  def writeSpaceFile(self, loopIDsToKeep):
    space = {}
    for loopID in self.ranges:
      if (loopID in loopIDsToKeep):
        space[loopID] = self.ranges[loopID]
      else:
        space[loopID] = [0, 0, 0, 0, 0, 0, 0, 0, 0]

    utils.writeConfFile(self.spaceFile, space)

    return


  def filter(self):
    techniqueIndex = 3
    loopIDsToKeep = []
    for loopID in self.ranges:
      conf = {}
      testingConf = False
      for loopIDToZeroOut in self.ranges:
        conf[loopIDToZeroOut] = [0, 0, 0, 0, 0, 0, 0, 0, 0]
        if (loopID == loopIDToZeroOut):
            if (self.ranges[loopIDToZeroOut][techniqueIndex] == 1): # Only DOALL enabled
              conf[loopIDToZeroOut] = [1, 0, 0, 4, 2, 8, 0, 0, 0]
              testingConf = True

      if (not testingConf):
        loopIDsToKeep.append(loopID)
        continue

      # Compile
      compileRetCode = utils.myCompile(self.confFile, conf)
      if (compileRetCode != 0):
        sys.exit(1)

      # Run parallel optimized binary
      maxExecutionTime = 2*self.baselineTime
      runRetCode = utils.myRun(maxExecutionTime)
      if (runRetCode != 0):
        sys.exit(1)

      # Get execution time
      time = utils.readExecutionTimeFile(self.executionTimeFile)
      tolerance = 1.2 # 20%
      if (time < (tolerance*self.baselineTime)):
        loopIDsToKeep.append(loopID)

    return loopIDsToKeep



if __name__ == '__main__':
  filterLoops = Filter()
  filterLoops.getArgs()
  loopIDsToKeep = filterLoops.filter()
  filterLoops.writeSpaceFile(loopIDsToKeep)
