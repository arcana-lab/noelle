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

thisPath = os.path.dirname(os.path.abspath(__file__))

sys.path.append(thisPath + "/../utils")
import utils

techniqueIndexConverter = [utils.Technique.DOALL, utils.Technique.HELIX, utils.Technique.DSWP]

class autotuneProgram(MeasurementInterface):
  ranges = None
  loopIDs = None
  confFile = None
  executionTimeFile = None
  exploredConfs = {}

  def getArgs(self):
    # Read the range of each dimension of the design space
    spaceFile = os.environ['autotunerSPACE_FILE']
    self.ranges, self.loopIDs = utils.readSpaceFile(spaceFile)

    # Get autotuner.info
    self.confFile = os.environ['INDEX_FILE']
    
    # Get execution time file
    self.executionTimeFile = os.environ['autotunerEXECUTION_TIME']

    self.baselineTimeFile = os.environ['autotunerBASELINE_TIME']
    self.baselineTime = utils.readExecutionTimeFile(self.baselineTimeFile)

    return


  def manipulator(self):
    """
    Define the search space by creating a
    ConfigurationManipulator
    """
    sys.stderr.write('AUTOTUNER: create the design space\n')

    self.getArgs()

    # Describe the design space to opentuner
    param = 0
    manipulator = ConfigurationManipulator()
    for loopID in self.loopIDs:
      for elem in self.ranges[loopID]:
        dimensionIsDigit = elem.isdigit()
        dimension = 0
        if (dimensionIsDigit):
          dimension = int(elem)

        # Check if the current dimension has a cardinality higher than 1
        if ((dimension > 1) or (not dimensionIsDigit)):

          # Check the type of the parameter
          paramType = param % 9

          # Create the parameter
          if (paramType == 0):
            start = 0
            stop = dimension - 1
            
            # Check if we want to force a specific parallelization technique
            forceDimension = len(elem.split("_")) == 2
            if (forceDimension):
              _, considerLoop = elem.split("_")
              start = int(considerLoop)
              stop = int(considerLoop)

            # Should the loop be parallelized?
            openTuner_param = IntegerParameter(str(param), start, stop)

          elif (paramType == 1):

            # Unroll factor
            openTuner_param = IntegerParameter(str(param), 0, dimension - 1)

          elif (paramType == 2):

            # Peel factor
            openTuner_param = IntegerParameter(str(param), 0, dimension - 1)

          elif (paramType == 3):
            start = 0
            stop = dimension - 1
            
            # Check if we want to force a specific parallelization technique
            forceDimension = len(elem.split("_")) == 2
            if (forceDimension):
              _, technique = elem.split("_")
              start = int(technique)
              stop = int(technique)

            # Parallelization technique
            openTuner_param = IntegerParameter(str(param), start, stop) # ED: should this be a SwitchParameter?

          elif (paramType == 4):

            # Number of cores to dedicate to the current loop
            openTuner_param = IntegerParameter(str(param), 0, dimension - 1)

          elif (paramType == 5):

            # DOALL parameter: chunk factor
            openTuner_param = IntegerParameter(str(param), 0, dimension - 1)

          elif (paramType == 6):

            # HELIX parameter: should we fix the maximum number of sequential segments?
            openTuner_param = SwitchParameter(str(param), dimension)

          elif (paramType == 7):

            # HELIX parameter: maximum number of sequential segments
            openTuner_param = IntegerParameter(str(param), 0, dimension - 1)

          elif (paramType == 8):

            # DSWP parameter: should we use queue packing?
            openTuner_param = SwitchParameter(str(param), dimension)

          # Share the parameter to OpenTuner
          manipulator.add_parameter(openTuner_param)

        param += 1

    return manipulator


  def getNormalizedConf(self, confArg):
    conf = confArg.copy()
    startLoopIndex = 0
    isLoopEnabled = False
    keysAsIntSorted = sorted([int(key) for key in confArg.keys()])
    for key in keysAsIntSorted:
      value = conf[str(key)]
      if ((key % 9) == 0):
        startLoopIndex = key
        if (value == 0): # Loop is disabled
          isLoopEnabled = False
          for keyToSetToZero in range(key, key + 9):
            if (str(keyToSetToZero) in conf):
              conf[str(keyToSetToZero)] = 0
        else:
          isLoopEnabled = True

      if (not isLoopEnabled):
        continue

      if ((key - startLoopIndex) == 3):
        conf[str(key)] = techniqueIndexConverter[value]
        if (conf[str(key)] != utils.Technique.DOALL): # DOALL was not chosen
          chunkSizeIndex = key + 2
          conf[str(chunkSizeIndex)] = 0

    return conf


  def getExpandedConf(self, confArg):
    conf = confArg.copy()
    index = 0
    for loopID in self.loopIDs:
      for elem in range(0, len(self.ranges[loopID])):
        if (str(index) not in conf):
          conf[str(index)] = 0
        index += 1

    return conf
 

  def getConfWithLoopIDs(self, confArg):
    conf = {}
    for loopID in self.loopIDs:
      conf[loopID] = []

    loopIDIndex = -1
    keysAsIntSorted = sorted([int(key) for key in confArg.keys()])
    for key in keysAsIntSorted:
      if ((key % 9) == 0):
        loopIDIndex += 1
      loopID = self.loopIDs[loopIDIndex]
      conf[loopID].append(confArg[str(key)])

    return conf
 

  def getConfAsStr(self, confArg):
    confAsStr = ''
    keysAsIntSorted = sorted([int(key) for key in confArg.keys()])
    for key in keysAsIntSorted:
      confAsStr += str(confArg[str(key)]) + '_'

    return confAsStr


  def run(self, desired_result, input, limit):
    """
    Compile and run a given configuration then
    return performance
    """

    # Read the configuration to run
    conf = desired_result.configuration.data
    sys.stderr.write("AUTOTUNER: conf " + str(conf) + "\n")
    confNormalized = self.getNormalizedConf(conf)
    sys.stderr.write("AUTOTUNER: confNormalized " + str(confNormalized) + "\n")
    confExpanded = self.getExpandedConf(confNormalized)
    sys.stderr.write("AUTOTUNER: confExpanded " + str(confExpanded) + "\n")

    confExpandedAsStr = self.getConfAsStr(confExpanded)
    sys.stderr.write("AUTOTUNER: confExpandedAsStr " + str(confExpandedAsStr) + "\n")
    time = None
    # Check if configuration has already been run
    if (confExpandedAsStr in self.exploredConfs):
      time = self.exploredConfs[confExpandedAsStr]
      return Result(time = time)
   
    try:
      # Compile
      confWithLoopIDs = self.getConfWithLoopIDs(confExpanded)
      sys.stderr.write("AUTOTUNER: confWithLoopIDs " + str(confWithLoopIDs) + "\n")
      compileRetCode = utils.myCompile(self.confFile, confWithLoopIDs)
      if (compileRetCode != 0):
        time = float('inf')
        return Result(time = time)

      # Run parallel optimized binary
      maxExecutionTime = 2*self.baselineTime
      runRetCode = utils.myRun(maxExecutionTime)
      if (runRetCode != 0):
        time = float('inf')
        return Result(time = time)

    except KeyboardInterrupt:
      sys.stderr.write("AUTOTUNER: KeyboardInterrupt. Abort.\n")
      sys.exit(1)

    # Get execution time
    time = utils.readExecutionTimeFile(self.executionTimeFile)

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
    confWithLoopIDs = self.getConfWithLoopIDs(confExpanded)
    compileRetCode = utils.myCompile(self.confFile, confWithLoopIDs)
    if (compileRetCode != 0):
      sys.stderr.write("AUTOTUNER: final configuration " + confExpandedAsStr + " did not compile.\nAbort.")
      sys.exit(1)
    
    # Dump explored configurations as json
    self.writeJson("exploredConfs.json", self.exploredConfs)

    return



if __name__ == '__main__':
  argparser = opentuner.default_argparser()
  autotuneProgram.main(argparser.parse_args())
