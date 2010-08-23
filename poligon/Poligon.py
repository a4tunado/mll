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

def process_task(task, executable):
  """Executes algorithm and returns poligon result"""
  
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
  
  logger.info(
        ('Processing task PocketId:{0},'
        + 'AlgSynonim:{1},'
        + 'ProblemSynonim:{2}...').format(task.PocketId
                                          , task.AlgSynonim
                                          , task.ProblemSynonim))
                                          
  results = []
  
  # loading all indexes, properties and data for given task
  params, indexes, data, properties, classes = \
      prepare_data(task, alg.password, os.path.dirname(executable))    
    
  for index in indexes:
    result = Result()
    # for i in range(properties):
    #   result.Test.PropertiesWeights.append(1.0)
    #   result.Learn.PropertiesWeights.append(1.0)
    # Executing algorithm 
    
    if not executable:
      result.Error = True
      result.ErrorException = 'Compile error'
    else:
      args = [os.path.relpath(executable)
              , alg.name, alg.author
              , os.path.relpath(params)
              , os.path.relpath(data)
              , os.path.relpath(index.learn.file)
              , os.path.relpath(index.test.file)
              , str(task.PocketId)]
      logger.info('Executing...')
      for i in range(len(args)):
        logger.debug(args[i])
      
      errtxt, outtxt = '', ''
      stderr = open(os.path.join(os.path.dirname(executable),'stderr.txt'),'w')
      stdout = open(os.path.join(os.path.dirname(executable),'stdout.txt'),'w')
      try:       
        popen = subprocess.Popen(args, stderr=stderr, stdout=stdout)
        if not _wait_process(popen):           
            result.Error = True
            result.ErrorException = 'Process execution timeout'
            logger.error(result.ErrorException)
        stderr.close()
        stderr = open(os.path.join(os.path.dirname(executable),'stderr.txt'))
        errtxt = stderr.read().decode('utf-8','replace').strip('\n')
        stdout.close()
        stdout = open(os.path.join(os.path.dirname(executable),'stdout.txt'))        
        outtxt = stdout.read().decode('utf-8','replace').strip('\n')
      except:
        logger.error('Unhandled exception occured while executing alg process')
      finally:
        stderr.close()
        stdout.close()
        
      if outtxt:
        logger.debug(outtxt)
      
    if not result.Error:
      logger.debug('Processed exited with code {0}'.format(popen.returncode))      
      if popen.returncode:          
        result.Error = True
        result.ErrorException = \
          'Process exited with error code: {0}\n{1}'.format(popen.returncode, errtxt)
        logger.error(result.ErrorException)
      elif errtxt:
        logger.error(errtxt)        
      
    test_targets = index.test.file.replace('.txt', '_TGT.txt')
    learn_targets = index.learn.file.replace('.txt', '_TGT.txt')
    if (os.path.exists(test_targets)):
      # Loading test targets
      result.Test.Targets = _load_vector(test_targets, int)
    else:
      error_exception = 'Test targets file was not found'
      if not result.Error:
        result.Test.Error = True
        result.Test.ErrorException = error_exception
      logger.error(error_exception)
      result.Test.Targets = [0 for i in range(index.test.count)]        
    if (os.path.exists(learn_targets)):
      # Loading learn targets
      result.Learn.Targets = _load_vector(learn_targets, int)
    else:
      error_exception = 'Learn targets file was not found'
      if not result.Error:
        result.Learn.Error = True
        result.Learn.ErrorException = error_exception
      logger.error(error_exception)
      result.Learn.Targets = [0 for i in range(index.learn.count)]
          
    # for i in range(len(result.Learn.Targets)):
    #  result.Learn.ObjectsWeights.append(1.0)
    
    # for i in range(len(result.LearnData.Targets)):
    #   result.TestData.ObjectsWeights.append(1.0)
          
    test_prob = index.test.file.replace('.txt', '_PRB.txt')
    learn_prob = index.learn.file.replace('.txt', '_PRB.txt')      
    if (os.path.exists(test_prob)):
      # Loading test prob matrix
      result.Test.ProbabilityMatrix = _load_matrix(test_targets, int)
    else:
      logger.warn('Test probability matrix was not found')
      for target in result.Test.Targets:
        vector = [0 for i in range(len(classes))]
        for i in range(len(classes)):
          if classes[i] == target:
            vector[i] = 1
        result.Test.ProbabilityMatrix.append(vector)      
    if (os.path.exists(learn_prob)):
      # Loading learn prob matrix
      result.Learn.ProbabilityMatrix = _load_matrix(learn_targets, int)
    else:
      logger.warn('Learn probability matrix was not found')
      for target in result.Learn.Targets:
        vector = [0 for i in range(len(classes))]
        for i in range(len(classes)):
          if classes[i] == target:
            vector[i] = 1
        result.Learn.ProbabilityMatrix.append(vector)
          
    results.append(result)                    # Saving targets
    
  logger.info('{0} targets produced'.format(len(results)))
    
  return results
 
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
  
def _wait_process(popen):
  spent = 0
  while popen.returncode == None:        
    time.sleep(WAIT_SLEEP)
    spent += WAIT_SLEEP
    if spent > WAIT_TIMEOUT:
      popen.kill()
      return False
    popen.poll()
  return True
  
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

  formatter = logging.Formatter("%(asctime)s\t%(levelname)s\t%(message)s")

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
  parser.add_option("-d", "--dataFile", dest="dataFile", default="data.arff",
                    help="Received data file path")
  parser.add_option("-l", "--learningIndexes", dest="learningIndexes", default="learningIndexes.txt",
                    help="Leraning indexes file path")
  parser.add_option("-t", "--testIndexes", dest="testIndexes", default="testIndexes.txt",
                    help="Test indexes file path")
  parser.add_option("-p", "--penalies", dest="penalties", default="penalties.txt",
                    help="Penalty matrix file path")
  parser.add_option("-r", "--algProperties", dest="algProperties", default="algProperties.txt",
                    help="Classifier properties output file path")
  parser.add_option("-T", "--targetOutput", dest="targetOutput", default="targetOutput.txt",
                    help="Target output file path")
  parser.add_option("-C", "--confidenceOutput", dest="confidenceOutput", default="confidenceOutput.txt",
                    help="Confidence output file path")
  parser.add_option("-F", "--featureWeightsOutput", dest="featureWeightsOutput", default="featureWeightsOutput.txt",
                    help="Feature weights output file path")
  parser.add_option("-W", "--objectWeightsOutput", dest="objectWeightsOutput", default="objectWeightsOutput.txt",
                    help="Object weights output file path")
  return parser

if __name__ == '__main__':

  logger = _init_logger()
  parser = _init_optparser()
  
  logger.info('Program started...')
 
  (options, args) = parser.parse_args()
  logger.info(options)

  task = poligon.get_task(options.algSynonim, options.algPassword)
  indexes = load_task_data(task, options.algPassword
                               , options.dataFile
                               , options.learnIndexes
                               , options.testIndexes
                               , options.algProperties)

  #TODO: execute algorithm with 


  #TODO: post results


  results = process_task(task, alg)
          task_processed = True
          if results:
            logger.info('Registering results \'{0}\'....'.format(task.PocketId))
            poligon.register_results(alg.algsynonim
                                    , alg.password
                                    , task.PocketId
                                    , results)
          time.sleep(PROCESS_SLEEP)
        else:
          logger.info('No  
