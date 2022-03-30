import os
import sys
import getpass

## Process the command line arguments
#
#
def getArgs():
  numOfArgs = len(sys.argv)
  args = {}

  args['fromFile'] = str(sys.argv[1])
  args['toFile'] = str(sys.argv[2])
  args['testsPath'] = str(sys.argv[3])
  args['noelleOptions'] = str(sys.argv[4])
  args['parOptions'] = str(sys.argv[5])
  args['FrontEndOptions'] = str(sys.argv[6])
  args['PreMiddleEndOptions'] = str(sys.argv[7])
  args['ToolOptions'] = str(sys.argv[8])

  args['email'] = str(getpass.getuser()) + '@eecs.northwestern.edu'
  if (numOfArgs > 9):
    args['email'] = str(sys.argv[9])

  repoPath = ''
  for elem in str(os.path.dirname(os.path.abspath(__file__))).split(os.sep)[1:-2]:
    repoPath += os.sep + elem
  args['repoPath'] = repoPath

  return args

def getNewFile(args):
  newFileAsStr = ''
  with open(args['fromFile'], 'r') as f:
    for line in f:
      if (line.startswith('Notify_User')):
        newFileAsStr += 'Notify_User = ' + args['email'] + '\n'
      elif (line.startswith('RepoPath')):
        newFileAsStr += 'RepoPath = ' + args['repoPath'] + '\n'
      elif (line.startswith('TestsPath')):
        newFileAsStr += 'TestsPath = ' + args['testsPath'] + '\n'
      elif (line.startswith('ParallelizationOptions')):
        newFileAsStr += 'ParallelizationOptions = ' + args['parOptions'] + '\n'
      elif (line.startswith('NoelleOptions')):
        newFileAsStr += 'NoelleOptions = ' + args['noelleOptions'] + '\n'
      elif (line.startswith('FrontEndOptions')):
        newFileAsStr += 'FrontEndOptions = ' + args['FrontEndOptions'] + '\n'
      elif (line.startswith('PreMiddleEndOptions')):
        newFileAsStr += 'PreMiddleEndOptions = ' + args['PreMiddleEndOptions'] + '\n'
      elif (line.startswith('ToolOptions')):
        newFileAsStr += 'ToolOptions = ' + args['ToolOptions'] + '\n'
      else:
        newFileAsStr += str(line)

  return newFileAsStr

def setNewFile(pathToFile, data):
  with open(str(pathToFile), 'w') as f:
    f.write(str(data))
    f.close()

  return

args = getArgs()
newFileAsStr = getNewFile(args)
setNewFile(args['toFile'], newFileAsStr)

