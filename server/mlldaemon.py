import os
import re
import sys
import time
import codecs
import logging
import subprocess
import distutils
from distutils import ccompiler
from datetime import datetime

import mlldb
import poligon
#import test.poligon as poligon

COMPILE_VERBOSE = 2

COMPILE_ARGS    = ['-O2',]

if os.name == 'nt':
  COMPILE_ARGS.append('/EHs')
  
MACROS          = ['NDEBUG',]

if os.name == 'nt':
  MACROS.append('_CRT_SECURE_NO_WARNINGS')

INCLUDE_DIRS    = ['../core',]
            
COMMON_SOURCES  = ['../core/dataset.cpp',
                  '../core/dataset_wrapper.cpp',
                  '../core/fix_alloc.cpp',
                  '../core/metadata.cpp',
                  '../core/metadata_wrapper.cpp',
                  '../core/util.cpp',
                  '../server/main.cpp',
                  '../core/tester.cpp',
                  '../core/cross_validation.cpp']
                      
LIBRARIES       = []

if os.name == 'posix':
  LIBRARIES.append('stdc++')  

BINARY_DIR      = '../server/bin'
                  
OBJECTS_DIR     = '../server/obj'

WAIT_TIMEOUT    = 15*60 # Alg execution wait timeout, seconds

WAIT_SLEEP      = 1     # Alg exection polling timeout, seconds

PROCESS_SLEEP   = 1     # Tasks requests timeout, seconds

WAKEUP_PERIOD   = 15*60 # seconds

logger = logging

def wakeup():
  """Retreives tasks from poligon and post results"""
  logger.info('Daemon woked up...')
  task_processed = False
  algs = mlldb.select_algs()
  for alg in algs:
    if alg.algsynonim and alg.password:
      if not os.path.exists(alg.path):
        logger.error(
          'Alg \'{0}\' path \'{1}\' doesn\'t exist'.format(alg.name, alg.path))
      else:
        task = poligon.get_task(alg.algsynonim, alg.password)
        if task:
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
          logger.info('No task to process')
  return task_processed
      
def process_task(task, alg):
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
      
  executable = build_alg(os.path.dirname(alg.path))
  
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
 
def build_alg(alg_path):
  """Builds algorithm executable"""
  logger.info('Compiling path \'{0}\'...'.format(alg_path))
  compiler = ccompiler.new_compiler(verbose=COMPILE_VERBOSE)
  
  if not os.path.exists(OBJECTS_DIR):
    os.mkdir(OBJECTS_DIR)
    
  if not os.path.exists(BINARY_DIR):
    os.mkdir(BINARY_DIR)

  for macro in MACROS:
    compiler.define_macro(macro)

  compiler.add_include_dir(alg_path)
  for dir in INCLUDE_DIRS:
    compiler.add_include_dir(dir)
    
  sources = []
  for root, dirs, files in os.walk(alg_path):
    for name in files:
      if re.match(r'.*\.c$|.*\.cpp$|.*\.cc$', name):
        name = os.path.abspath(os.path.join(root, name))
        if not name in sources:
          sources.append(name)
    if '.svn' in dirs:
      dirs.remove('.svn')  # don't visit SVN directories

  for name in COMMON_SOURCES:
    name = os.path.abspath(name)
    if not name in sources:
      sources.append(name)  
  
  today = datetime.today()
  prefix = today.strftime('%Y%m%d_%H%M%S_%f').replace(',','')
  
  bin_path = os.path.abspath(os.path.join(BINARY_DIR, prefix))
  obj_path = os.path.abspath(os.path.join(OBJECTS_DIR, prefix))
  
  if os.path.exists(bin_path):
    logger.warn('Path \'{0}\' is already exists'.format(bin_path))
    os.rmdir(bin_path)
    
  if os.path.exists(obj_path):
    logger.warn('Path \'{0}\' is already exists'.format(obj_path))
    os.rmdir(obj_path)
  
  os.mkdir(bin_path)
  os.mkdir(obj_path)
  
  logger.debug('Compiling sources:')
  for name in sources:
    logger.debug(name)
  
  try:
    objects = compiler.compile(sources
                             , output_dir=obj_path
                             , debug=0
                             , extra_preargs=COMPILE_ARGS)
  except:
    logger.error(
      'Compilation error occured \'{0}\'\n{1}'.format(alg_path, sys.exc_info()[0]))
    return None

  logger.debug('Linking:')
  for name in objects:
    logger.debug(name)     

  executable = os.path.join(bin_path, prefix)

  try:
    compiler.link_executable(objects, executable, libraries=LIBRARIES)
  except:
    logger.error(
      'Linker error occured \'{0}\'\n{1}'.format(executable, sys.exc_info()[0]))
    return None
      
  executable = compiler.executable_filename(executable)                  
                          
  logger.info(
    'Path \'{0}\' is compiled successfully: \'{1}\''.format(alg_path, executable))
  
  return executable
  
def prepare_data(task, password, path):
  """Writes problem data, indexes and alg params into the given path"""  
  
  class Indexes(object):
        
    def __init__(self, learn, test):
      self.test = test
      self.learn = learn
  
  problem = poligon.get_problem(task.ProblemSynonim, task.AlgSynonim, password)
  
  if problem:
    logger.info(
      'Problem \'{0}\' received with {1} rows'.format(
        task.ProblemSynonim, len(problem.DataMatrix.ArrayOfDouble)))
  else:
    logger.error(
      'Can\'t receive problem \'{0}\''.format(task.ProblemSynonim))
    return  
  
  today = datetime.today()
  prefix = today.strftime('%Y%m%d_%H%M%S_%f').replace(',','')
  prefix = os.path.join(path, prefix)
  
  # Writing problem file
  data = prefix + '.arff'
  objects, properties, classes = _save_as_arff(problem, task.ProblemSynonim, data)
  logger.info('Problem data saved as \'{0}\''.format(data))  
  
  # Writing indexes into separate files
  test = _save_indexes(task.TestIndexes, prefix + '_TST')
  logger.info('{0} test indexes saved'.format(len(test)))
  
  learn = _save_indexes(task.LearnIndexes, prefix + '_LRN')  
  logger.info('{0} learn indexes saved'.format(len(learn)))
  
  if len(test) != len(learn):
    logger.error('Indexes count mismatch')
    
  indexes = []
  for i in range(min(len(learn),len(test))):
    indexes.append(Indexes(learn[i], test[i]))
    
  # Writing alg params
  params = prefix + '.cfg'
  count = _save_params(task, params)
  logger.info('{0} alg params saved as \'{1}\''.format(count, params))
  
  return params, indexes, data, properties, classes

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
  
def _save_indexes(indexes, prefix):
  """Writes indexes into separate files"""
  
  class Index(object):
    
    def __init__(self):
      self.file = ''
      self.count = 0

  items = []
  for i in range(len(indexes.ArrayOfInt)):
    index = Index()
    index.file = '{0}_{1}.txt'.format(prefix, i) 
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
  return matrix
  
def _update_repo():
  """Updates algorithms from svn repo and synchronizes db"""
  logger.info('Updating svn repo \'{0}\'...'.format(mlldb.ALGS_PATH))
  args = ['svn','update',mlldb.ALGS_PATH]
  popen = subprocess.Popen(args)
  popen.wait()
  mlldb.load_algs(mlldb.ALGS_PATH)
  
if __name__ == '__main__':

  today = datetime.today()  
  logger = logging.getLogger('Daemon')  
  logger.setLevel(logging.DEBUG)
  
  stream_handler = logging.StreamHandler()
  
  file_handler = logging.FileHandler(
    today.strftime('daemon_%Y%m%d_%H%M%S.log')
    , encoding='utf-8')
  
  formatter = logging.Formatter("%(asctime)s\t%(levelname)s\t%(message)s")
  stream_handler.setFormatter(formatter)
  file_handler.setFormatter(formatter)
  
  logger.addHandler(stream_handler)
  logger.addHandler(file_handler)
  
  mlldb.logger = logger
  poligon.logger = logger
  
  mlldb.sync()
  
  while True:
    try:
      _update_repo()
      while wakeup():
        pass
    except:
      logger.error('Unhandled error occured\n{0}'.format(sys.exc_info()[0]))
    time.sleep(WAKEUP_PERIOD)
   
