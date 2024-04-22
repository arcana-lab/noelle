import os
import sys
import json
import tempfile

thisPath = os.path.dirname(os.path.abspath(__file__))

sys.path.append(thisPath + "/../utils")
import utils

# DOALL index is 4
reverseIndex = 4

# We should consider all permutations, but they can grow quickly, and we can't spend all autotuner time running seed configurations.
#reverseTechniqueIndexConverter = [[utils.Technique.DOALL-reverseIndex,utils.Technique.HELIX-reverseIndex,utils.Technique.DSWP-reverseIndex], [utils.Technique.DOALL-reverseIndex,utils.Technique.HELIX-reverseIndex], [utils.Technique.DOALL-reverseIndex,utils.Technique.DSWP-reverseIndex], [utils.Technique.HELIX-reverseIndex,utils.Technique.DSWP-reverseIndex], [utils.Technique.DOALL-reverseIndex], [utils.Technique.HELIX-reverseIndex], [utils.Technique.DSWP-reverseIndex]]

# So here's an heuristic: if we have multiple choices we tend to pick DOALL
reverseTechniqueIndexConverter = [utils.Technique.DOALL-reverseIndex, utils.Technique.DOALL-reverseIndex, utils.Technique.DOALL-reverseIndex, utils.Technique.HELIX-reverseIndex, utils.Technique.DOALL-reverseIndex, utils.Technique.HELIX-reverseIndex, utils.Technique.DSWP-reverseIndex]

def readJson(pathToFile):
  data = {}
  with open(str(pathToFile)) as f:    
    data = json.load(f)
    f.close()

  return data

def writeJson(pathToFile, jsonData):
  with open(pathToFile, 'w') as f:
    json.dump(jsonData, f)
    f.close()

  return

def readSeedConf(pathToFile):
  techniqueToDisableIndex = 3
  seedConf = {}
  with open(str(pathToFile), 'r') as f:
    i = 0
    for line in f:
      lineAsList = line.split()
      sys.stderr.write(str(lineAsList) + "\n")
      # If loop is disabled, then skip the remaining parameters
      if (int(lineAsList[1]) == 0):
        i += len(lineAsList[1:])
        continue
      
      # Skip the loop ID for the seed configuration
      for elem in lineAsList[1:]:
        value = int(elem)

        if (((i - techniqueToDisableIndex) % 9) == 0):
          seedConf[i] = reverseTechniqueIndexConverter[value]
        else:
          if (value != 0):
            seedConf[i] = value

        i += 1
    f.close()
 
  sys.stderr.write(str(seedConf) + "\n")
  return seedConf

def genSeedConfFile(seedConfJson):
  newfile, pathToFile = tempfile.mkstemp(suffix = '.json')
  writeJson(pathToFile, seedConfJson)

  return pathToFile

def genSeedConf(pathToSeedConf):
  seedConfJson = readSeedConf(pathToSeedConf)
  pathToFile = genSeedConfFile(seedConfJson)

  return pathToFile


if __name__ == '__main__':
  pathToSeedConf = genSeedConf(sys.argv[1])
  print(pathToSeedConf)

