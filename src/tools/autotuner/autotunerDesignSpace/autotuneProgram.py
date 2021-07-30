#!/usr/bin/env python
import os
import sys
import socket
import importlib
import traceback
import adddeps  # fix sys.path
import array
import numpy as np
import time

import opentuner
from opentuner import ConfigurationManipulator
from opentuner import IntegerParameter
from opentuner import LogIntegerParameter
from opentuner import SwitchParameter
from opentuner import MeasurementInterface
from opentuner import Result

repoPath = os.environ['REPO_PATH']
sys.path.append(repoPath + '/scripts')

import utils
import profiler

class autotuneProgram(MeasurementInterface):

  # Fields
  args = None
  finalConfFlag = False
  configurationsFileName = 'configurations.json'
  crashedConfigurationsFileName = 'crashedConfigurations.json'
  configurationFileName = 'configuration.txt'
  outputDistortionModule = None
  designSpace = array.array('i')
  configurationsExplored = 0
  configurationsRun = 0

  def manipulator(self):
    """
    Define the search space by creating a
    ConfigurationManipulator
    """
    sys.stderr.write('AUTOTUNER:   Create the design space\n')

    # Check if manipulator has already been invoked
    defineDesignSpace = len(self.designSpace) == 0

    # Read the range of each dimension of the design space
    inputFile = os.environ['RANGE_FILE']
    ranges = utils.readFile(inputFile)

    # Describe the design space to opentuner
    param = 0
    manipulator = ConfigurationManipulator()
    for elem in ranges:

      # Check if there is a design space
      if defineDesignSpace:
        self.designSpace.append(elem)

      # Check if the current dimension has a cardinality higher than 1
      if elem > 1:

        # Check the type of the parameter
        paramType = param % 9

        # Create the parameter
        if paramType == 0:

          # Should the loop be parallelized?
          openTuner_param = SwitchParameter(str(param), elem)

        elif paramType == 1:

          # Unroll factor
          openTuner_param = IntegerParameter(str(param), 0, str(elem - 1))

        elif paramType == 2:

          # Peel factor
          openTuner_param = IntegerParameter(str(param), 0, str(elem - 1))

        elif paramType == 3:

          # Parallelization technique
          openTuner_param = IntegerParameter(str(param), 0, str(elem - 1))

        elif paramType == 4:

          # Number of cores to dedicate to the current loop
          openTuner_param = IntegerParameter(str(param), 0, str(elem - 1))

        elif paramType == 5:

          # DOALL parameter: chunk factor
          openTuner_param = IntegerParameter(str(param), 0, str(elem - 1))

        elif paramType == 6:

          # HELIX parameter: should we fix the maximum number of sequential segments?
          openTuner_param = SwitchParameter(str(param), elem)

        elif paramType == 7:

          # HELIX parameter: maximum number of sequential segments
          openTuner_param = IntegerParameter(str(param), 0, str(elem - 1))

        elif paramType == 8:

          # DSWP parameter: should we use queue packing?
          openTuner_param = SwitchParameter(str(param), elem)


        # Share the parameter to OpenTuner
        manipulator.add_parameter(openTuner_param)

      param += 1

    # Load outputDistortion module if possible
    benchmark = os.environ['BENCHMARK_NAME']
    self.outputDistortionModule = None
    try:
      self.outputDistortionModule = importlib.import_module('benchmark.' + benchmark + '.outputDistortion')
    except ImportError:
      self.outputDistortionModule = None

    if defineDesignSpace:
      sys.stderr.write('AUTOTUNER:     Whole design space (' + str(len(self.designSpace))+ ') = ' + str(self.designSpace) + '\n')

    return manipulator


  def run(self, desired_result, input, limit):
    result = None
    """
    Compile and run a given configuration then
    return performance
    """

    # Read the configuration to run
    cfg = desired_result.configuration.data
    if (not cfg): # no ranges
      sys.exit(0)
      return

    # Translate the configuration into a string representation
    cfgStr = self.getCfgStr(cfg)
    self.configurationsExplored += 1
    sys.stderr.write('AUTOTUNER:     Configuration ' + cfgStr + '\n')

    configurations, pathToFiles = self.readConfigurations()
    if (cfgStr in configurations):

      # The configuration has been executed. Return the previous results
      result = configurations[cfgStr]
      sys.stderr.write('AUTOTUNER:       This configuration has already been evaluated: ' + str(result) + '\n')
      time.sleep(5)

    else:
      crashedFlag = False
      sys.stderr.write('AUTOTUNER:         Configuration ' + str(self.configurationsRun) + ' to test (explored by the autotuner ' + str(self.configurationsExplored - 1) + ')\n')
      self.configurationsRun += 1

      # Generate the binary
      didWeCompileSuccesfully = self.myCompile(cfg, 0)

      # Check if the compiler succeded
      if didWeCompileSuccesfully == 0:

        # Invoke the profiler to test the current configuration
        try:
  
          # Define the output directory
          pathToOutputDir = self.getInfoStr()
          pathToOutputDir += '/' + str(self.configurationsExplored)
  
          # Run
          result = profiler.profile(pathToOutputDir)
  
        except KeyboardInterrupt:
          #raise
          result = self.getResultForCrash()
          sys.exit(1)
        except Exception as e:
          sys.stderr.write('AUTOTUNER: ERROR = ' + str(e) + '\n')
          traceback.print_exc()
          result = self.getResultForCrash()
          self.writeConfigurations(cfgStr, result, os.path.join(pathToFiles, self.crashedConfigurationsFileName))
          crashedFlag = True
  
        # The configuration run. Compute the output distortion
        sys.stderr.write('AUTOTUNER:             Binary run\n')
        outputDistortionFlag = bool(int(os.environ['OUTPUT_DISTORTION_FLAG']))
        isDistorted = False
        if (('outputDistortion' in result) and (outputDistortionFlag)):
          inputName = os.environ['INPUT_NAME']
          isDistorted = self.outputDistortionModule.isDistorted(inputName, result['outputDistortion'])
          if (isDistorted):
            sys.stderr.write('AUTOTUNER:                 The output is distorted\n')
            result['experimentTime'] = float('inf')
  
        # Dump the results
        if ((not crashedFlag) and (not isDistorted)):
          self.writeConfigurations(cfgStr, result, os.path.join(pathToFiles, self.configurationsFileName))
          self.createConfigurationFile(pathToOutputDir + '/' + self.configurationFileName, cfgStr)
  
        # Print results
        sys.stderr.write('AUTOTUNER:                 Time = ' + str(result['experimentTime']) + '\n')


      else:

        # The compiler failed
        sys.stderr.write('AUTOTUNER:           The compiler failed\n')
        result = self.getResultForCrash()


    return Result(time=result['experimentTime'])


  def createConfigurationFile(self, pathToFile, dataAsStr):
    with open(str(pathToFile), 'w') as f:
      f.write(dataAsStr)
      f.close()

    return


  def save_final_config(self, configuration):
    """called at the end of tuning"""
    sys.stderr.write('AUTOTUNER:   The autotuning loop is over\n')

    self.finalConfFlag = True
    configurations, _ = self.readConfigurations()
    bestConfiguration, bestExecutionTime = self.getBestConfiguration(configurations)
    self.myCompile(bestConfiguration, bestExecutionTime)

    # Print the best configuration
    cfg = configuration.data
    cfgStr = self.getCfgStr(cfg)
    sys.stderr.write('AUTOTUNER:   After ' + str(self.configurationsRun) + ' configurations run and ' + str(self.configurationsExplored) + ' configurations explored, the best one is ' + cfgStr + '\n')
    sys.stderr.write('AUTOTUNER: Exit\n')
    
    return


  def getArgs(self):
    hostname = socket.gethostname()
    args = {}
    args['argsSet'] = os.environ['ARGS_TMP']

    configPath = os.environ['REPO_PATH'] + '/config/' + hostname + '.json'
    configJson = utils.readJson(configPath)
    args['configJson'] = configJson

    benchmark = os.environ['BENCHMARK_NAME']
    args['benchmark'] = benchmark

    numOfCores = int(os.environ['NUM_OF_CORES'])
    numOfIterations = int(os.environ['MAX_ITERATIONS'])
    htFlag = bool(int(os.environ['HT_FLAG']))
    pwFlag = bool(int(os.environ['PW_FLAG']))
    args['configBenchmarkJson'] = {'numOfCores': numOfCores, 'numOfIterations': numOfIterations, 'HT': htFlag, 'PW': pwFlag}

    return args


  def getInfoStr(self):
    inputName = os.environ['INPUT_NAME']

    hostname = socket.gethostname()

    repoPath = os.environ['REPO_PATH']
    benchmarkName = os.environ['BENCHMARK_NAME']

    numOfCores = int(os.environ['NUM_OF_CORES'])
    htFlag = bool(int(os.environ['HT_FLAG']))
    pwFlag = bool(int(os.environ['PW_FLAG']))

    modeStr = 'REG'
    if (htFlag):
      modeStr = 'HT'
    if (pwFlag):
      modeStr += '/PW'

    pathToFiles = repoPath + '/data/' + benchmarkName + '/' + hostname + '/' + modeStr + '/' + str(numOfCores) + '/' + inputName

    return pathToFiles   


  def getCfgStr(self, cfg):

    # Fetch the configuration
    indexes = utils.getIndexesOfLength(cfg, len(self.designSpace) - 1)

    # Erase parameters that do not make sense
    self.eraseUselessParameters(indexes)

    # Define the configuration expanded to also include dimensions with cardinality of 1
    cfgStr = ''
    param = 0
    for index in indexes:

      # Append the current dimension specified in the configuration 
      cfgStr += str(index) + '_'
      param += 1

    # Safety check
    if param != len(self.designSpace):
      sys.stderr.write('AUTOTUNER: Error = the design space is not properly expanded. Param \"' + str(param) + '\" is not equal to the size of the design space, which is ' + str(len(self.designSpace)) + '\n')
      sys.exit(1)

    return cfgStr


  def getResultForCrash(self):
    result = {}
    result['experimentTime'] = float('inf')

    return result


  def writeConfigurations(self, cfgStr, result, pathToConfigurationsFile):
    configurations = {}
    if (os.path.exists(pathToConfigurationsFile)):
      configurations = utils.readJson(pathToConfigurationsFile)
    configurations[cfgStr] = result
    utils.writeJson(pathToConfigurationsFile, configurations)

    return


  def readConfigurations(self):
    pathToFiles = self.getInfoStr()
    configurations = {}
    self.args = self.getArgs()
    configJson = self.args['configBenchmarkJson']
    pathToConfigurationsFile = os.path.join(pathToFiles, self.configurationsFileName)
    if (os.path.exists(pathToConfigurationsFile)):
      configurations = utils.readJson(pathToConfigurationsFile)

    return configurations, pathToFiles


  def getCfgDict(self, cfg):
    cfgAsDict = {}
    i = 0
    for elem in cfg.split('_')[:-1]:
      cfgAsDict[str(i)] = int(elem)
      i += 1

    return cfgAsDict


  def getBestConfiguration(self, configurations):
    if (len(configurations) > 0):
      bestExecutionTime = configurations[configurations.keys()[0]]['experimentTime']
      bestConfiguration = configurations.keys()[0]
      for k in configurations:
        if (configurations[k]['experimentTime'] < bestExecutionTime):
          bestExecutionTime = configurations[k]['experimentTime']
          bestConfiguration = k
      bestConfigurationAsDict = self.getCfgDict(bestConfiguration)
      return bestConfigurationAsDict, bestExecutionTime
    else:
      sys.stderr.write('ERROR: no best configuration')
      sys.exit(1)


  def myCompile(self, cfg, executionTime):

    # Fetch the configuration
    indexes = utils.getIndexesOfLength(cfg, len(self.designSpace) - 1)

    # Erase parameters that do not make sense
    self.eraseUselessParameters(indexes)

    # Write the configuration to the INDEX_FILE file
    inputName = os.environ['INPUT_NAME']
    outputFile = os.environ['INDEX_FILE']
    utils.writeFile(outputFile, indexes)

    repoPath = os.environ['REPO_PATH']
    if (self.finalConfFlag):
      args = self.args

      hostname = socket.gethostname()
      benchmarkName = args['benchmark']

      configJson = args['configBenchmarkJson']
      numOfCores = configJson['numOfCores']
      htFlag = configJson['HT']
      pwFlag = configJson['PW']

      modeStr = 'REG'
      if (htFlag):
        modeStr = 'HT'
      if (pwFlag):
        modeStr += '/PW'

      bestConfFile = repoPath + '/data/' + benchmarkName + '/' + hostname + '/' + modeStr + '/' + str(numOfCores) + '/' + inputName + '/bestConfiguration.json'
      bestConf = {'conf': indexes, 'time': executionTime}
      utils.writeJson(bestConfFile, bestConf)

    return os.system(repoPath + '/src/scripts/backEnd') # generate the binary of the best configuration found


  def eraseUselessParameters(self, indexes):

    # Erase parallelization parameters related to loops chosen to stay sequential
    param = 0
    for index in indexes:
      paramID = param % 9
      if paramID == 0:
        reset = 0

      # Erase parallelization parameters related to loops chosen to stay sequential
      if paramID == 0 and index == 0:
        reset = 1

      if reset == 1:
        indexes[int(param)] = 0

      param += 1

    # Erase parameters currently not used
    param = 0
    for index in indexes:
      paramID = param % 9
      reset = 0

      # Is loop Parallelized?
      if paramID == 0:
        doallIsDisabled = 0

      # Unroll 
      if paramID == 1:
        reset = 1

      # Peel
      if paramID == 2:
        reset = 1

      # Parallelization techniques disabled
      # 0: None
      # 1: DSWP
      # 2: HELIX
      # 3: DOALL
      # 4: DSWP, HELIX
      # 5: DSWP, DOALL
      # 6: HELIX, DOALL
      if paramID == 3:
        if index == 3 or index == 5 or index == 6:
          doallIsDisabled = 1

      # DOALL: chunk factor
      if paramID == 5 and doallIsDisabled == 1:
        reset = 1

      # Unused
      if paramID == 6:
        reset = 1
      if paramID == 7:
        reset = 1
      if paramID == 8:
        reset = 1

      # Reset
      if reset == 1:
        indexes[int(param)] = 0

      param += 1


if __name__ == '__main__':
  argparser = opentuner.default_argparser()
  autotuneProgram.main(argparser.parse_args())
