import os
import sys
import logging
from datetime import datetime
from ZSI import ParsedSoap, SoapWriter

_path = os.path.split(os.path.dirname(os.path.abspath(__file__)))[0]
if not _path in sys.path:
  sys.path.append(_path)

from ProcessingService_client import *
from ProcessingService_types import *

#URL = 'http://poligon.machinelearning.ru/TestService.asmx'
URL = 'http://poligon.machinelearning.ru/ProcessingService.asmx'

TODAY = datetime.today()

TRACE_FILE = open(TODAY.strftime('trace_%Y%m%d_%H%M%S.xml'), 'w')

KW = { 'tracefile' : TRACE_FILE }

TASK_FILE = ''

PROBLEM_FILE = ''

REGISTER_RESULTS = False

service = ProcessingServiceSoapSOAP(URL, **KW)

logger = logging



def get_task(algsynonim, password):
  
  if TASK_FILE:
    file = open(TASK_FILE)
    data = file.read()
    file.close()    
    ps = ParsedSoap(data)
    return ps.Parse(GetTaskSoapOut.typecode).GetTaskResult
  else:
    """Requests poligon task"""
    logger.info('Requesting task for \'{0}\'...'.format(algsynonim))
    task_request = GetTaskSoapIn()
    task_request.Password = password
    task_request.AlgSynonim = algsynonim

    try:
      response = service.GetTask(task_request, **KW)
      if (not response.GetTaskResult
          or not response.GetTaskResult.LearnIndexes):
        return None
    except:
      logger.error(
        'Error occured while requesting task for \'{0}\'\n{1}'.format(algsynonim, sys.exc_info()[0]))
      return None
      
    return response.GetTaskResult
  
def get_problem(probsynonim, algsynonim, password):

  if PROBLEM_FILE:
    file = open(PROBLEM_FILE)
    data = file.read()
    file.close()  
    ps = ParsedSoap(data)
    return ps.Parse(GetProblemSoapOut.typecode).GetProblemResult
  else:
    """Requests poligon for a given problem"""
    logger.info('Requesting problem \'{0}\'...'.format(probsynonim))
    problem_request = GetProblemSoapIn()
    problem_request.Password = password
    problem_request.AlgSynonim = algsynonim
    problem_request.ProblemSynonim = probsynonim

    try:
      response = service.GetProblem(problem_request, **KW)
      if not len(response.GetProblemResult.DataMatrix.ArrayOfDouble):
        return None
    except:
      logger.error(
        'Error occured while requesting problem \'{0}\'\n{1}'.format(probsynonim, sys.exc_info()[0]))
      return None
      
    return response.GetProblemResult
  
def register_results(algsynonim, password, pocketid, results):
  """Posts results to the poligon server"""
  register_request = RegisterResultSoapIn()
  register_request.AlgSynonim = algsynonim
  register_request.Password = password
  register_request.PocketKey = int(pocketid)
  
  register_request.LearnResults = register_request.new_learnResults()
  register_request.TestResults = register_request.new_testResults()
  
  for i in range(len(results)):
    
    test_result = register_request.TestResults.new_TestResult()
    learn_result = register_request.LearnResults.new_TestResult()
    
    test_result.Index = i
    learn_result.Index = i
    
    test_result.Error = results[i].Error
    test_result.ErrorException = results[i].ErrorException
    learn_result.Error = results[i].Error
    learn_result.ErrorException = results[i].ErrorException
    
    if not results[i].Error:
      test_result.Error = results[i].Test.Error
      test_result.ErrorException = results[i].Test.ErrorException
      learn_result.Error = results[i].Test.Error
      learn_result.ErrorException = results[i].Test.ErrorException
   
    # Answers
    test_result.Answers = test_result.new_Answers()
    learn_result.Answers = learn_result.new_Answers()
    
    for j in range(len(results[i].Learn.Targets)):
      learn_result.Answers.Int.append(int(results[i].Learn.Targets[j]))
    
    for j in range(len(results[i].Test.Targets)):
      test_result.Answers.Int.append(int(results[i].Test.Targets[j]))
     
    # PropertiesWeights 
    #test_result.PropertiesWeights = test_result.new_PropertiesWeights()
    #learn_result.PropertiesWeights = learn_result.new_PropertiesWeights()
    
    #for j in range(len(results[i].LearnPropertiesWeights)):
    #  learn_result.PropertiesWeights.Double.append(results[i].LearnPropertiesWeights[j])
    
    #for j in range(len(results[i].TestPropertiesWeights)):
    #  test_result.PropertiesWeights.Double.append(results[i].TestPropertiesWeights[j])
    
    # Object weights
    #test_result.ObjectsWeights = test_result.new_ObjectsWeights()
    #learn_result.ObjectsWeights = learn_result.new_ObjectsWeights()
    
    #for j in range(len(results[i].LearnObjectsWeights)):
    #  learn_result.ObjectsWeights.Double.append(results[i].LearnObjectsWeights[j])
    
    #for j in range(len(results[i].TestObjectsWeights)):
    #  test_result.ObjectsWeights.Double.append(results[i].TestObjectsWeights[j])
    
    # ProbabilityMatrix
    test_result.ProbabilityMatrix = test_result.new_ProbabilityMatrix()
    learn_result.ProbabilityMatrix = learn_result.new_ProbabilityMatrix()
    
    for vector in results[i].Test.ProbabilityMatrix:
      array = test_result.ProbabilityMatrix.new_ArrayOfDouble()
      for item in vector:
        array.Double.append(item)
      test_result.ProbabilityMatrix.ArrayOfDouble.append(array)
    
    for vector in results[i].Learn.ProbabilityMatrix:
      array = learn_result.ProbabilityMatrix.new_ArrayOfDouble()
      for item in vector:
        array.Double.append(item)
      learn_result.ProbabilityMatrix.ArrayOfDouble.append(array)      
    
    register_request.TestResults.TestResult.append(test_result)   
    register_request.LearnResults.TestResult.append(learn_result)

  try:
    sw = SoapWriter()
    sw.serialize(register_request)
    
    file = open('RegisterResult.xml', 'w')
    file.write(str(sw))
    file.close()
  except:
    logger.error('Can\'t write file results file')
  
  if REGISTER_RESULTS:
    try:
      response = service.RegisterResult(register_request, **KW)
      if response.RegisterResultResult.Status == 'Ok':
        logger.info('Results are posted successfully')
      else:
        logger.error(
          'Error occured while posting results\n{0}'.format(response.RegisterResultResult.Message.encode('unicode', 'replace')))
      return response.RegisterResultResult 
    except:
      logger.error(
        'Error occured while posting results \'{0}\'\n{1}'.format(pocketid, sys.exc_info()[0]))
    
  return None
  
  
if __name__ == '__main__':
  print get_problem('UCI_Hepatitis','ys.shestopalov.knn','orsi7bt6')
  pass

