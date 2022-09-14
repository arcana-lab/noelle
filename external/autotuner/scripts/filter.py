#!/usr/bin/env python

import os
import sys
import json
import traceback

thisPath = os.path.dirname(os.path.abspath(__file__))

class Filter:
  spaceFile = None
  confFile = None
  executionTimeFile = None
  ranges = None

  def getArgs(self):
    self.spaceFile = os.environ['autotunerSPACE_FILE']
    self.confFile = os.environ['INDEX_FILE']
    self.executionTimeFile = os.environ['autotunerEXECUTION_TIME']

    self.ranges = self.readSpaceFile(self.spaceFile)

    return


  def readExecutionTimeFile(self, pathToFile):
    lineAsFloat = None
    with open(str(pathToFile), 'r') as f:
      line = f.readline()
      lineAsFloat = float(line)
      f.close()

    return lineAsFloat


  def readSpaceFile(self, pathToFile):
    ranges = {}
    with open(str(pathToFile), 'r') as f:
      for line in f.readlines():
        loopID = int(line.split()[0])
        ranges[loopID] = []
        for elem in line.split()[1:]:
          ranges[loopID].append(int(elem))
      f.close()

    return ranges


  def writeConfFile(self, pathToFile, conf):
    strToWrite = ""
    for loopID in conf:
      strToWrite += str(loopID)
      for elem in conf[loopID]:
        strToWrite += " " + str(elem)
      strToWrite += "\n"

    with open(str(pathToFile), "w") as f:
      f.write(strToWrite)
      f.close()

    return


  def writeSpaceFile(self, loopIDsToKeep):
    space = {}
    for loopID in self.ranges:
      if (loopID in loopIDsToKeep):
        space[loopID] = self.ranges[loopID]
      else:
        space[loopID] = [0, 0, 0, 0, 0, 0, 0, 0, 0]

    self.writeConfFile(self.spaceFile, space)

    return


  def compileAndRunBaseline(self):
    conf = {}
    for loopID in self.ranges:
      conf[loopID] = []
      for elem in self.ranges[loopID]:
        conf[loopID].append(0)

    # Compile
    compileRetCode = self.myCompile(conf)
    if (compileRetCode != 0):
      sys.exit(1)

    # Run parallel optimized binary
    runRetCode = self.myRun()
    if (runRetCode != 0):
      sys.exit(1)

    # Get execution time
    time = self.readExecutionTimeFile(self.executionTimeFile)

    return time


  def filter(self):
    timeBaseline = self.compileAndRunBaseline()

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
      compileRetCode = self.myCompile(conf)
      if (compileRetCode != 0):
        sys.exit(1)

      # Run parallel optimized binary
      runRetCode = self.myRun()
      if (runRetCode != 0):
        sys.exit(1)

      # Get execution time
      time = self.readExecutionTimeFile(self.executionTimeFile)

      if (time < timeBaseline):
        loopIDsToKeep.append(loopID)

    return loopIDsToKeep


  def myCompile(self, conf):
    # Write autotuner.info file
    self.writeConfFile(self.confFile, conf)

    return os.system(thisPath + '/../scripts/compile')


  def myRun(self):
    return os.system(thisPath + '/../scripts/run')



if __name__ == '__main__':
  filterLoops = Filter()
  filterLoops.getArgs()
  loopIDsToKeep = filterLoops.filter()
  filterLoops.writeSpaceFile(loopIDsToKeep)
