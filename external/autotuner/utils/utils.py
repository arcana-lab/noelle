#!/usr/bin/env python

import os
import sys

# Risky. It works because autotuner.py and filter.py are at the same level in the directoy tree.
thisPath = os.path.dirname(os.path.abspath(__file__))


def readExecutionTimeFile(pathToFile):
  lineAsFloat = None
  with open(str(pathToFile), 'r') as f:
    line = f.readline()
    lineAsFloat = float(line)
    f.close()

  return lineAsFloat


def readSpaceFile(pathToFile):
  ranges = {}
  loopIDs = []
  with open(str(pathToFile), 'r') as f:
    for line in f.readlines():
      loopID = int(line.split()[0])
      loopIDs.append(loopID)
      ranges[loopID] = []
      for elem in line.split()[1:]:
        ranges[loopID].append(elem)
    f.close()

  return ranges, loopIDs


def writeConfFile(pathToFile, conf):
  strToWrite = ""
  for loopID in conf:
    strToWrite += str(loopID)
    for elem in conf[loopID]:
      strToWrite += " " + str(elem)
    strToWrite += "\n"

  with open(str(pathToFile), 'w') as f:
    f.write(strToWrite)
    f.close()

  return


def myCompile(confFile, conf):
  # Write autotuner_conf.info file
  writeConfFile(confFile, conf)

  return os.system(thisPath + "/../scripts/compile")


def myRun(maxExecutionTime = 0):
  retcode = 0
  if (maxExecutionTime == 0):
    retcode = os.system(thisPath + "/../scripts/run")
  else:
    retcode = os.system("timeout " + str(maxExecutionTime) + "s" + " " + thisPath + "/../scripts/run")

  return retcode
