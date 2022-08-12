#!/usr/bin/env python

import os
import sys
import json
import traceback

import opentuner
from opentuner import ConfigurationManipulator
from opentuner import IntegerParameter
from opentuner.search.manipulator import BooleanParameter
from opentuner import LogIntegerParameter
from opentuner import SwitchParameter
from opentuner import MeasurementInterface
from opentuner import Result

from enum import Enum

thisPath = os.path.dirname(os.path.abspath(__file__))

class Technique(Enum):
  DOALL = 4
  HELIX = 5
  DSWP = 6

techniqueIndexConverter = [Technique.DOALL, Technique.HELIX, Technique.DSWP]

class autotuneProgram(MeasurementInterface):
  ranges = None
  confFile = None
  executionTimeFile = None
  exploredConfs = {}

  def getArgs(self):
    # Read the range of each dimension of the design space
    spaceFile = os.environ['autotunerSPACE_FILE']
    self.ranges = self.readSpaceFile(spaceFile)

    # Get autotuner.info
    self.confFile = os.environ['INDEX_FILE']
    
    # Get execution time file
    self.executionTimeFile = os.environ['autotunerEXECUTION_TIME']

    return


  def readExecutionTimeFile(self, pathToFile):
    lineAsFloat = None
    with open(str(pathToFile), 'r') as f:
      line = f.readline()
      lineAsFloat = float(line)
      f.close()
  
    return lineAsFloat


  def readSpaceFile(self, pathToFile):
    ranges = []
    with open(str(pathToFile), 'r') as f:
      for line in f.readlines():
        for elem in line.split():
          ranges.append(int(elem))
      f.close()
  
    return ranges


  def writeConfFile(self, pathToFile, conf):
    strToWrite = ''
    for key in sorted(conf.keys()):
      if ((key % 9) == 0) and (key != 0):
        strToWrite += '\n'
      strToWrite += str(conf[key]) + ' '

    with open(str(pathToFile), 'w') as f:
      f.write(strToWrite)
      f.close()
  
    return


  def manipulator(self):
    """
    Define the search space by creating a
    ConfigurationManipulator
    """
    sys.stderr.write('AUTOTUNER:   Create the design space\n')

    self.getArgs()

    # Describe the design space to opentuner
    param = 0
    manipulator = ConfigurationManipulator()
    for elem in self.ranges:
      # Check if the current dimension has a cardinality higher than 1
      if elem > 1:

        # Check the type of the parameter
        paramType = param % 9

        # Create the parameter
        if paramType == 0:
          # Should the loop be parallelized?
          openTuner_param = SwitchParameter(param, elem)
          #openTuner_param = BooleanParameter(param)

        elif paramType == 1:

          # Unroll factor
          openTuner_param = IntegerParameter(param, 0, elem - 1)

        elif paramType == 2:

          # Peel factor
          openTuner_param = IntegerParameter(param, 0, elem - 1)

        elif paramType == 3:

          # Parallelization technique
          openTuner_param = IntegerParameter(param, 0, elem - 1) # ED: should this be a SwitchParameter?

        elif paramType == 4:

          # Number of cores to dedicate to the current loop
          openTuner_param = IntegerParameter(param, 0, elem - 1)

        elif paramType == 5:

          # DOALL parameter: chunk factor
          openTuner_param = IntegerParameter(param, 0, elem - 1)

        elif paramType == 6:

          # HELIX parameter: should we fix the maximum number of sequential segments?
          openTuner_param = SwitchParameter(param, elem)

        elif paramType == 7:

          # HELIX parameter: maximum number of sequential segments
          openTuner_param = IntegerParameter(param, 0, elem - 1)

        elif paramType == 8:

          # DSWP parameter: should we use queue packing?
          openTuner_param = SwitchParameter(param, elem)

        # Share the parameter to OpenTuner
        manipulator.add_parameter(openTuner_param)

      param += 1

    return manipulator


  def getNormalizedConf(self, confArg):
    conf = confArg.copy()
    startLoopIndex = 0
    isLoopEnabled = False
    for key in conf:
      value = conf[key]
      if ((key % 9) == 0):
        startLoopIndex = key
        if (value == 0): # Loop is disabled
          isLoopEnabled = False
          for keyToSetToZero in range(key, key + 9):
            if (keyToSetToZero in conf):
              conf[keyToSetToZero] = 0
        else:
          isLoopEnabled = True

      if (not isLoopEnabled):
        continue

      if (key - startLoopIndex) == 3:
        conf[key] = techniqueIndexConverter[value]
        if (value != Technique.DOALL): # DOALL was not chosen
          chunkSizeIndex = key + 2
          conf[chunkSizeIndex] = 0

    return conf


  def getExpandedConf(self, confArg):
    conf = confArg.copy()
    index = 0
    for elem in self.ranges:
      if (index not in conf):
        conf[index] = 0
      index += 1

    return conf
 

  def myCompile(self, confExpanded):
    # Write autotuner.info file
    self.writeConfFile(self.confFile, confExpanded)

    return os.system(thisPath + '/../scripts/compile')


  def myRun(self):
    return os.system(thisPath + '/../scripts/run')


  def getConfAsStr(self, conf):
    confAsStr = ''
    for key in sorted(conf.keys()):
      confAsStr += str(conf[key]) + '_'

    return confAsStr


  def run(self, desired_result, input, limit):
    """
    Compile and run a given configuration then
    return performance
    """

    # Read the configuration to run
    conf = desired_result.configuration.data
    confNormalized = self.getNormalizedConf(conf)
    confExpanded = self.getExpandedConf(confNormalized)

    confExpandedAsStr = self.getConfAsStr(confExpanded)
    time = None
    # Check if configuration has already been run
    if (confExpandedAsStr in self.exploredConfs):
      time = self.exploredConfs[confExpandedAsStr]
      return Result(time = time)
    
    # Compile
    compileRetCode = self.myCompile(confExpanded)
    if (compileRetCode != 0):
      time = float('inf')
      return Result(time = time)

    # Run parallel optimized binary
    runRetCode = self.myRun()
    if (runRetCode != 0):
      time = float('inf')
      return Result(time = time)

    # Get execution time
    time = self.readExecutionTimeFile(self.executionTimeFile)

    # Save conf in our list of explored configurations
    self.exploredConfs[confExpandedAsStr] = time

    return Result(time=time)


  def writeJson(self, pathToFile, jsonData):
    with open(pathToFile, 'w') as f:
      json.dump(jsonData, f)
      f.close()

    return


  def save_final_config(self, configuration):
    # Generate bitcode and binary with final configuration
    conf = configuration.data
    confNormalized = self.getNormalizedConf(conf)
    confExpanded = self.getExpandedConf(confNormalized)
    confExpandedAsStr = self.getConfAsStr(confExpanded)
    compileRetCode = self.myCompile(confExpanded)
    if (compileRetCode != 0):
      sys.stderr.write("AUTOTUNER: final configuration " + confExpandedAsStr + " did not compile.\nAbort.")
      sys.exit(1)
    
    # Dump explored configurations as json
    self.writeJson("exploredConfs.json", self.exploredConfs)

    return



if __name__ == '__main__':
  argparser = opentuner.default_argparser()
  autotuneProgram.main(argparser.parse_args())
