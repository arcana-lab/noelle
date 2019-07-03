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

  args['email'] = str(getpass.getuser()) + '@eecs.northwestern.edu'
  if (numOfArgs > 3):
    args['email'] = str(sys.argv[3])

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

