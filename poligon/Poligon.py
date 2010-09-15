import os
import re
import sys
import time
import codecs
import logging
import subprocess
from optparse import OptionParser
from datetime import datetime

import PoligonService as poligon
#import test.poligon as poligon

logger = logging

def load_task_data(task, algPassword, dataFile, learnIndexes, testIndexes, algProperties):
  """Writes problem data, indexes and alg params into the given path"""  
  
  class Indexes(object):
        
    def __init__(self, learn, test):
      self.test = test
      self.learn = learn
 
  # 1. Loading problem data 
  problem = poligon.get_problem(task.ProblemSynonim, task.AlgSynonim, algPassword)

  if not problem:
    logger.error(
      'Can\'t load problem \'{0}\' from server'.format(task.ProblemSynonim))
  
  logger.debug(
    'Problem \'{0}\' received with {1} rows'.format(
      task.ProblemSynonim, len(problem.DataMatrix.ArrayOfDouble)))
  
  objects, properties, classes = _save_as_arff(problem, task.ProblemSynonim, dataFile)
  logger.debug('Problem data saved as \'{0}\''.format(dataFile))  
  
  # 2. Saving indexes
  test = _save_indexes(task.TestIndexes, testIndexes)
  logger.debug('{0} test indexes saved'.format(len(test)))
  
  learn = _save_indexes(task.LearnIndexes, learnIndexes)  
  logger.info('{0} learn indexes saved'.format(len(learn)))
  
  if len(test) != len(learn):
    logger.error('Indexes count mismatch')
    
  indexes = []
  for i in range(0, len(learn)):
    indexes.append(Indexes(learn[i], test[i]))
    
  # 3. Writing alg properties
  count = _save_params(task, algProperties)
  logger.info('{0} alg params saved as \'{1}\''.format(count, algProperties))
  
  return indexes #, properties, classes

def _save_as_arff(problem, synonim, path):
  """Writes problem data in arff format"""
  classes = []
  
  file = open(path, 'w')
  file.write('%\n%\n%\n\n')  
  file.write('@RELATION {0}\n\n'.format(synonim))
  
  attributes = []
  width = len(problem.PropertiesDescription.PropertyDescription)
  
  for i in range(width-1):
    nominals = {}
    property = problem.PropertiesDescription.PropertyDescription[i]   
    attribute = '@ATTRIBUTE A{0}\t'.format(i+1)
    if property.Type == 'Nominal':
      attribute = attribute + '{'
      for j in range(len(property.Values.Int)):
        tag = 'A{0}{1}'.format(i+1,j+1)
        nominals[int(property.Values.Int[j])] = tag
        attribute = attribute + '{0},'.format(tag)
      attribute = attribute.rstrip(',') + '}'
    else:
      attribute = attribute + 'REAL'
    attribute = attribute + '\n'
    file.write(attribute)
    attributes.append(nominals)
  
  attribute = '@ATTRIBUTE class\t{'  
  property = problem.PropertiesDescription.PropertyDescription[width-1]
  for value in property.Values.Int:
    classes.append(value)
    attribute = attribute + '{0},'.format(value)
  attribute = attribute.rstrip(',') + '}'
  attribute = attribute + '\n'  
  file.write(attribute)
    
  file.write('\n@DATA\n')
  
  for i in range(len(problem.DataMatrix.ArrayOfDouble)):
    row = ''
    for j in range(len(problem.DataMatrix.ArrayOfDouble[i].Double)):
      data = str(problem.DataMatrix.ArrayOfDouble[i].Double[j])
      if data.lower() == 'nan':
        data = '?'
      elif attributes[j]:
        data = attributes[j][int(problem.DataMatrix.ArrayOfDouble[i].Double[j])]
      row = row + '{0},'.format(data)
    row = row + '{0}\n'.format(problem.Target.Int[i])
    file.write(row)
    
  file.write('%\n%\n%\n\n')
  file.close()
  
  return len(problem.DataMatrix.ArrayOfDouble) \
    , len(problem.PropertiesDescription.PropertyDescription) \
    , classes
  
def _save_indexes(indexes, path):
  """Writes indexes into separate files"""
  
  class Index(object):
    
    def __init__(self):
      self.file = ''
      self.count = 0

  (root, ext) = os.path.splitext(path)

  items = []
  for i in range(len(indexes.ArrayOfInt)):
    index = Index()
    index.file = '{0}_{1}{2}'.format(root, i, ext) 
    index.count = len(indexes.ArrayOfInt[i].Int)   
    file = open(index.file, 'w')
    for j in range(len(indexes.ArrayOfInt[i].Int)):
      file.write('{0}\n'.format(indexes.ArrayOfInt[i].Int[j]))
    file.close()    
    items.append(index)
  return items
  
def _save_params(task, path):
  """Writes alg params into the given file"""
  # NOTE: parameter name and value shouldn't contain any blanks
  count = 0
  file = open(path, 'w')
  try:
    if task.AlgParamNames:
      for i in range(len(task.AlgParamNames.String)):
        if task.AlgParamUsages.Boolean[i]:
          count += 1
          file.write('{0}={1}\n'.format(
                      task.AlgParamNames.String[i]
                      , task.AlgParamValues.String[i]))
  except:
    logger.error('Error occured while saving params\n{0}'.format(sys.exc_info()[0]))
  file.write('\n')
  file.close()
  return count

def _load_vector(name, type=str):
  vector = []
  file = open(name)
  text = file.read()
  file.close()
  lines = text.split('\n')
  for line in lines:
    if (len(line)):
      vector.append(type(line))
  return vector
  
def _load_matrix(name, type=str):
  matrix = []
  file = open(name)
  text = file.read()
  file.close()
  lines = text.split('\n')
  for line in lines:
    if (len(line)):
      items = line.split(' ')
      vector = []
      for item in items:
        vector.append(type(item))
      matrix.append(vector)

def _init_logger():
  today = datetime.today()
  logger = logging.getLogger('Daemon')
  logger.setLevel(logging.DEBUG)

  stream_handler = logging.StreamHandler()

  file_handler   = logging.FileHandler(
					today.strftime('poligon_%Y%m%d_%H%M%S.log')
					, encoding='utf-8')

  formatter = logging.Formatter(
					"%(asctime)s\t%(levelname)s\t%(message)s")

  stream_handler.setFormatter(formatter)
  file_handler.setFormatter(formatter)

  logger.addHandler(stream_handler)
  logger.addHandler(file_handler)

  poligon.logger = logger

  return logger

def _init_optparser():
  parser = OptionParser()
  parser.add_option("-c", "--classifier", dest="classifier",
                    help="Classifier name")
  parser.add_option("-e", "--executable", dest="executable", default="mll",
                    help="Executable file path")
  parser.add_option("-S", "--algSynonim", dest="algSynonim",
                    help="Executable file path")
  parser.add_option("-P", "--algPassword", dest="algPassword",
                    help="Executable file path")
  parser.add_option("-d", "--data", dest="data", default="data.arff",
                    help="Received data file path")
  parser.add_option("-l", "--learnIndexes", dest="learnIndexes", default="learnIndexes.txt",
                    help="Learn indexes file path")
  parser.add_option("-t", "--testIndexes", dest="testIndexes", default="testIndexes.txt",
                    help="Test indexes file path")
  #parser.add_option("-p", "--penalies", dest="penalties", default="penalties.txt",
  #                  help="Penalty matrix file path")
  parser.add_option("-r", "--algProperties", dest="algProperties", default="algProperties.txt",
                    help="Classifier properties output file path")
  parser.add_option("-A", "--learnTargetOutput", dest="learnTargetOutput", default="learnTarget.txt",
                    help="Learn targets output file path")
  parser.add_option("-T", "--testTargetOutput", dest="testTargetOutput", default="testTarget.txt",
                    help="Test targets output file path")
  parser.add_option("-N", "--learnProbOutput", dest="learnProbOutput", 
                    help="Learn probabilyties output file path")
  parser.add_option("-R", "--testProbOutput", dest="testProbOutput",
                    help="Test probabilyties output file path")
  #parser.add_option("-C", "--confidenceOutput", dest="confidenceOutput", default="confidenceOutput.txt",
  #                  help="Confidence output file path")
  #parser.add_option("-F", "--featureWeightsOutput", dest="featureWeightsOutput", default="featureWeightsOutput.txt",
  #                  help="Feature weights output file path")
  #parser.add_option("-W", "--objectWeightsOutput", dest="objectWeightsOutput", default="objectWeightsOutput.txt",
  #                  help="Object weights output file path")
  #parser.add_option("-L", "--logFile", dest="logFile", default="poligon.log",
  #                  help="Log file ouput path")
  #parser.add_option("-V", "--logLevel", dest="logLevel", default="0xf",
  #					help="Log level mask")
  return parser

if __name__ == '__main__':

  class Result(object):

    class Data(object):

      def __init__(self):
        self.Error = False
        self.ErrorException = ''
        self.ProbabilityMatrix = []
        self.Targets = []
        self.PropertiesWeights = []
        self.ObjectsWeights = []

    def __init__(self):
      self.Error = False
      self.ErrorException = ''
      self.Test = Result.Data()
      self.Learn = Result.Data()

  logger = _init_logger()
  parser = _init_optparser()
  
  (options, args) = parser.parse_args()
  logger.info(options)

  # 1. Loading task info from poligon server
  task = poligon.get_task(options.algSynonim
							, options.algPassword)

  # 2. Loading task data and creating input files
  indexes = load_task_data(task, options.algPassword
                            , options.data
                            , options.learnIndexes
                            , options.testIndexes
                            , options.algProperties)

  results = []

  for index in indexes:
    result = Result()
    # 3. Executing algorithm
    args = [options.executable
			, "classify"
			, "--classifier"        , options.classifier     
			, "--data"			    , options.data  
			, "--trainIndexes"		, index.learn.file
            , "--testIndexes"		, index.test.file
			, "--trainTargetOutput"	, options.learnTargetOutput
			, "--testTargetOutput"  , options.testTargetOutput]
    
    logger.info('Executing {0}...'.format(options.executable))
    logger.debug(str(args))

    try:
      popen = subprocess.Popen(args)
      popen.wait()
    except:
      result.Error = True
      result.ErrorException = \
        'Error occured while executing process: {0}'.format(sys.exc_info()[0])
      logger.error(result.ErrorException)
      results.append(result)
      continue

    # 4. Handling errors
    if popen.returncode:
      result.Error = True
      result.ErrorException = \
        'Process exited with error code: {0}'.format(popen.returncode)
      logger.error(result.ErrorException)
      results.append(result)
      continue

    # 5. Loading results

    result.Test.Targets = _load_vector(options.testTargetOutput, int)
    result.Learn.Targets = _load_vector(options.learnTargetOutput, int)

    # TODO: Generate probability matrix if it is not present

    if options.testProbOutput:
      result.Test.ProbabilityMatrix = _load_matrix(options.testProbOutput, int)
      
    if options.learnTargetOutput:
      result.Learn.ProbabilityMatrix = _load_matrix(options.learnTargetOutput, int)

    results.append(result)                    # Saving targets

  logger.info('{0} targets produced'.format(len(results)))

  # 6. Registering results
  poligon.register_results(options.algSynonim
							, options.algPassword
                            , task.PocketId
							, results)
    

