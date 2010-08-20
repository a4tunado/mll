#include <vector>
#include <string>
#include <algorithm>
#include <stdexcept>

#include <stdio.h>
#include <stdarg.h>

#include "sh_ptr.h"
#include "dataset.h"
#include "factories.h"
#include "classifier.h"
#include "dataset_wrapper.h"

#define USE_LOG  1
#define LOG_FILE "main.log"

using std::vector;
using std::string;

using namespace mll;

struct Args {

  Args(int argc, const char* argv[]);

  std::string alg_name_;
  std::string alg_author_;
  std::string alg_params_;
  std::string data_;
  std::string learn_;
  std::string test_;
  std::string pocket_id_;
};

class Log {
public:
	static Log& Instance();
	void Write(const char* format, ...);	
	void WriteError(const char* format, ...);
private:
	Log();
	~Log();
	char buf_[512];
	FILE* file_;
};

#if USE_LOG
# define LOG(format, ...) Log::Instance().Write(format, __VA_ARGS__)
# define LOGE(format, ...) Log::Instance().WriteError(format, __VA_ARGS__)
#else
# define LOG(format, ...) (void)0
# define LOGE(format, ...) (void)0
#endif

sh_ptr<DataSet> LoadDataSet(const string& dataFileName) {
    sh_ptr<DataSet> dataSet(new DataSet());
    if (!dataSet->Load(dataFileName)) {
        LOG("Error: dataset '%s' was not loaded correctly"
          , dataSet->GetName().c_str());
        return dataSet;
    }
    LOG("Dataset '%s' loaded", dataSet->GetName().c_str());
    LOG("Features: %d", dataSet->GetFeatureCount());
    LOG("Objects : %d", dataSet->GetObjectCount());
    LOG("Classes : %d", dataSet->GetClassCount());
    return dataSet;
}

sh_ptr<IClassifier> CreateClassifier(const string& classifierName
                                    , const string& classifierAuthor) {
    sh_ptr<IClassifier> classifier =
      ClassifierFactory::Instance().Create(classifierName, classifierAuthor);
    return classifier;
}

void LoadParams(const string& configFileName, sh_ptr<IClassifier>& classifier) {
  char name[64] = {0,};
  char line[128] = {0,};  
  LOG("Loading alg params '%s'...", configFileName.c_str());
  FILE* file = fopen(configFileName.c_str(), "r");
  if (!file) { LOG("Can't read file '%s'", configFileName.c_str()); return; }  
  while (fscanf(file, "%[^\n]s", line) != EOF) {
    if (strlen(line) > 3) {
      char* value = strstr(line, "=");
      if (value) {
        strncpy(name, line, value - line);
        value += 1;
        LOG("\t%s=%s", name, value);
        if (!classifier->SetParameter(name, value)) {
          LOG("\tParameter '%s' is not accepted", name);
        }
      }
      else {
        LOG("\tError: Delimeter was not found: %s", line);
      }
    }
    fscanf(file, "%c", line);
    line[1] = 0;
  }
  fclose(file);
}

void LoadIndexes(const string& indexesFileName, std::vector<int>* indexes) {
  int index = 0;
  FILE* file = fopen(indexesFileName.c_str(), "r");
  if (!file) { LOG("Can't read file '%s'", indexesFileName.c_str()); return; }
  indexes->clear();
  while (fscanf(file, "%d", &index) != EOF) {
    indexes->push_back(index);
  }
  fclose(file);
}

void TestClassifier(sh_ptr<IClassifier>& classifier
                    , sh_ptr<DataSet> dataSet
                    , const std::vector<int>& learningIndexes
                    , const std::vector<int>& testIndexes
                    , std::vector<int>* learningTargets
                    , std::vector<int>* testTargets) {
  
  DataSetWrapper testSet(dataSet.get());
  DataSetWrapper trainSet(dataSet.get());
                   
  trainSet.SetObjectIndexes(
    learningIndexes.begin(), learningIndexes.end());
    
  testSet.SetObjectIndexes(
    testIndexes.begin(), testIndexes.end());
      
  DataSetWrapper trainSetWrapper(&trainSet); 
    
  classifier->Learn(&trainSet); 
  for (int i = 0; i < trainSetWrapper.GetObjectCount(); ++i) {
    trainSetWrapper.SetTarget(i, Refuse);
  }   
  classifier->Classify(&trainSetWrapper); 
  trainSetWrapper.ResetObjectIndexes();
  learningTargets->resize(trainSetWrapper.GetObjectCount());
  for (int i = 0; i < trainSetWrapper.GetObjectCount(); ++i) {
    (*learningTargets)[i] = trainSetWrapper.GetTarget(i);
  }
  LOG("Learning targets produced: %d", learningTargets->size());
      
  DataSetWrapper testSetWrapper(&testSet);

  for (int i = 0; i < testSetWrapper.GetObjectCount(); ++i) {
    testSetWrapper.SetTarget(i, Refuse);
  }
  classifier->Classify(&testSetWrapper); 
  testSetWrapper.ResetObjectIndexes();
  testTargets->resize(testSetWrapper.GetObjectCount());
  for (int i = 0; i < testSetWrapper.GetObjectCount(); ++i) {
    (*testTargets)[i] = testSetWrapper.GetTarget(i);
  }
  LOG("Test targets produced: %d", testTargets->size());
}

void SaveTargets(const string& indexesFileName, const std::vector<int>& indexes) {
  FILE* file = fopen(indexesFileName.c_str(), "w");
  if (!file) { LOG("Can't write file '%s'", indexesFileName.c_str()); return; }
  for (int i = 0; i < (int)indexes.size(); i++) {
    fprintf(file, "%d\n", indexes[i]);
  }
  fclose(file);
}

string GetTargetsFileName(const string& indexFileName) {
  int index = indexFileName.find(".txt");  
  if (index == string::npos) {
    LOG("Incorrect file name '%s'", indexFileName.c_str());
    return "";
  }
  string targetFileName = indexFileName;
  targetFileName.replace(index, 4, "_TGT.txt");
  return targetFileName;
}

int main(int argc, const char* argv[]) {
  
  Args args(argc, argv);
  
  LOG("=====================================================================", 0);
  LOG("Alg Name     : %s", args.alg_name_.c_str());
  LOG("Alg Author   : %s", args.alg_author_.c_str());
  LOG("Alg Params   : %s", args.alg_params_.c_str());
  LOG("Data         : %s", args.data_.c_str());
  LOG("Learn Indexes: %s", args.learn_.c_str());
  LOG("Test Indexes : %s", args.test_.c_str());
  LOG("Pocket Id    : %s", args.pocket_id_.c_str());
  
  string testTargetsFileName = GetTargetsFileName(args.test_);
  string learningTargetsFileName = GetTargetsFileName(args.learn_);
  
  if (testTargetsFileName.empty()
      || learningTargetsFileName.empty()) {
    return EXIT_FAILURE;
  }
  
  sh_ptr<IClassifier> classifier = 
    CreateClassifier(args.alg_name_, args.alg_author_);
    
  if (classifier.get()) {
    LOG("Classifier '%s' is loaded", args.alg_name_.c_str());
  }
  else {
    LOGE("Classifier was not found",0);
    return EXIT_FAILURE;
  }
    
  sh_ptr<DataSet> dataSet = LoadDataSet(args.data_);
  
  if (!dataSet->GetObjectCount()) {
    LOGE("Error: dataset '%s' is empty", args.data_.c_str());
    return EXIT_FAILURE;
  }
  
  LoadParams(args.alg_params_, classifier);
  
  std::vector<int> testIndexes;
  std::vector<int> learningIndexes;
    
  LoadIndexes(args.learn_, &learningIndexes);  
  if (learningIndexes.empty()) {
    LOGE("Error: learn indexes are not loaded",0);
    return EXIT_FAILURE;
  }
  LOG("Learn indexes loaded: %d", learningIndexes.size());
  
  LoadIndexes(args.test_, &testIndexes);  
  if (testIndexes.empty()) {
    LOGE("Test indexes are not loaded",0);
    return EXIT_FAILURE;
  }
  LOG("Test indexes loaded: %d", testIndexes.size());
  
  {
    int max_learn = *std::max_element(
      learningIndexes.begin(), learningIndexes.end());
    int max_test = *std::max_element(
      testIndexes.begin(), testIndexes.end());
    
    LOG("Max learn index: %d", max_learn);
    LOG("Max test index: %d",max_test);
    
    if (max_learn >= dataSet->GetObjectCount()) {
      LOGE("Error: learn max index %d exceeds object count %d"
          , max_learn, dataSet->GetObjectCount());
      return EXIT_FAILURE;
    } 
    
    if (max_test >= dataSet->GetObjectCount()) {
      LOGE("Error: test max index %d exceeds object count %d"
          , max_test, dataSet->GetObjectCount());
      return EXIT_FAILURE;
    }
  }
  
  std::vector<int> testTargets;
  std::vector<int> learningTargets;
  
  try {
    LOG("Testing...",0);
    TestClassifier(classifier, dataSet
                  , learningIndexes
                  , testIndexes
                  , &learningTargets
                  , &testTargets);
    LOG("Classifier successfully tested",0);
  }
  catch (const std::exception& ex) {
    LOGE("%s",ex.what());    
    return EXIT_FAILURE;
  }
  catch (...) {
    LOGE("Unhandled exception occured",0);
    return EXIT_FAILURE;
  }
                
  SaveTargets(testTargetsFileName, testTargets);
  SaveTargets(learningTargetsFileName, learningTargets);
  
  LOG("Targets are saved",0);
  
  return EXIT_SUCCESS;
}

Args::Args(int argc, const char* argv[]) {
  if (argc > 1) { alg_name_.assign(argv[1]); }
  if (argc > 2) { alg_author_.assign(argv[2]); }
  if (argc > 3) { alg_params_.assign(argv[3]); }
  if (argc > 4) { data_.assign(argv[4]); }
  if (argc > 5) { learn_.assign(argv[5]); }
  if (argc > 6) { test_.assign(argv[6]); }
  if (argc > 7) { pocket_id_.assign(argv[7]); }
}

Log::Log()
	: file_(NULL) { }

Log::~Log() {
	if (file_) {
		fclose(file_);
	}
}

Log& Log::Instance() {
	static Log log; return log;
}

void Log::Write(const char* format, ...) {
	va_list args;
  va_start(args, format);
  vsprintf(buf_, format, args);
  va_end(args);
  //std::cout << buf_ << std::endl; 
	if (!file_) { file_ = fopen(LOG_FILE, "a"); }
	if (file_) {
		fprintf(file_, "%s\n", buf_);
		fflush(file_);
	}
}

void Log::WriteError(const char* format, ...) {
	va_list args;
  va_start(args, format);
  vsprintf(buf_, format, args);
  va_end(args);
  std::cerr << buf_ << std::endl; 
	if (!file_) { file_ = fopen(LOG_FILE, "a"); }
	if (file_) {
		fprintf(file_, "ERROR: %s\n", buf_);
		fflush(file_);
	}
}
