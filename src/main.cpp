#include <ctime>
#include <cstdio>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>

#include <tclap/CmdLine.h>

#include "classifier.h"
#include "cross_validation.h"
#include "dataset.h"
#include "dataset_wrapper.h"
#include "factories.h"
#include "tester.h"
#include "logger.h"

using std::cout;
using std::cerr;
using std::endl;
using std::list;
using std::vector;
using std::string;
using std::ifstream;

using namespace mll;

void ListClassifiers() {
    cout << "Registered classifiers:" << endl;
    vector<ClassifierFactory::Entry> classifiers;
    ClassifierFactory::Instance().GetEntries(&classifiers);
    for (vector<ClassifierFactory::Entry>::const_iterator it = classifiers.begin();
         it != classifiers.end();
         ++it) {
        cout << "\t" << it->GetName()
             << " (" << it->GetAuthor() << "): " 
             << it->GetDescription() << endl;
    }
    cout << endl;
}

void ListTesters() {
    cout << "Registered testers:" << endl;
    vector<TesterFactory::Entry> testers;
    TesterFactory::Instance().GetEntries(&testers);
    for (vector<TesterFactory::Entry>::const_iterator it = testers.begin();
         it != testers.end();
         ++it) {
        cout << "\t" << it->GetName()
             << " (" << it->GetAuthor() << "): " 
             << it->GetDescription() << endl;
    }
    cout << endl;
}

sh_ptr<IClassifier> CreateClassifier(const string& classifierName) {
    sh_ptr<IClassifier> classifier = ClassifierFactory::Instance().Create(classifierName);
    if (classifier.get() == NULL) {
		LOGF("Classifier '%s' is not registered", LOGSTR(classifierName));
    }
	LOGD("Classifier '%s' is loaded", LOGSTR(classifierName));
    return classifier;
}

sh_ptr<ITester> CreateTester(const string& testerName) {
    sh_ptr<ITester> tester = TesterFactory::Instance().Create(testerName);
    if (tester.get() == NULL) {
        LOGF("Tester '%s' is not registered", LOGSTR(testerName));
    }
	LOGD("Tester '%s' is loaded", LOGSTR(testerName));
    return tester;
}

sh_ptr<DataSet> LoadDataSet(const string& fileName) {
    sh_ptr<DataSet> dataSet(new DataSet());
    if (!dataSet->Load(fileName)) {
		LOGF("Can't load dataset %s", LOGSTR(fileName));
    }
    LOGI("DataSet '%s' is loaded:", LOGSTR(dataSet->GetName()));
    LOGI("Features: %d", dataSet->GetFeatureCount());
    LOGI("Objects : %d", dataSet->GetObjectCount());
    LOGI("Classes : %d", dataSet->GetClassCount());
    return dataSet;
}

template<typename T>
void LoadVector(const string& fileName, typename std::vector<T> *data) {
	std::ifstream input(fileName.c_str());
	if (!input.is_open()) {
		LOGF("Can't open input file: '%s'", LOGSTR(fileName));
	}
	data->clear();
	typename std::istream_iterator<T> it(input), end;
	std::copy(it, end, std::back_inserter(*data));
	LOGD("%d entries loaded from '%s'", data->size(), LOGSTR(fileName));
}

template<typename T>
void WriteVector(const std::string& fileName, const typename std::vector<T> &data) {
	std::ofstream output(fileName.c_str());
	if (!output.is_open()) {
		LOGF("Can't open output file: '%s'", LOGSTR(fileName));
	}
	for (typename std::vector<T>::const_iterator it = data.begin()
		; it != data.end(); ++it) {
		output << *it << std::endl;
	}
	LOGD("%d entries written to '%s'", data.size(), LOGSTR(fileName));
}

template<typename T>
void WriteMatrix(const std::string& fileName, const typename std::vector<T> &data, int columns) {
    std::ofstream output(fileName.c_str());
    if (!output.is_open()) {
        LOGF("Can't open output file: '%s'", LOGSTR(fileName));
    }	
    for (typename std::vector<T>::const_iterator it = data.begin()
        ; it != data.end(); ++it) {
		output << *it;
		if ((it + 1 - data.begin()) % columns) output << " ";
		else output << std::endl;
    }
    LOGD("%d x %d matrix written to '%s'", (data.size() / columns), columns, LOGSTR(fileName));
}

void Classify(const IClassifier& classifier
				, /*const*/ IDataSet* dataSet
				, std::vector<int>* targets
				, std::vector<float>* confidence = NULL) {
	targets->clear();
	targets->resize(dataSet->GetObjectCount(), Refuse);
	if (confidence) {
		// NOTE: Assume that targets is numeric sequence starting from 0
		classifier.Classify(dataSet, confidence);
		std::vector<int>::iterator target = targets->begin();
        for (std::vector<float>::iterator it = confidence->begin()
			; it != confidence->end(); it += dataSet->GetClassCount(), ++target) {
            std::vector<float>::iterator current =
				std::max_element(it, it + dataSet->GetClassCount());
            if (*current > 0.f) *target = current - it;                    
        }
	}
    else {
		DataSetWrapper wrapper(dataSet);
		for (int i = 0; i < wrapper.GetObjectCount(); ++i) {
			wrapper.SetTarget(i, Refuse);
        }
        classifier.Classify(&wrapper);
        wrapper.ResetObjectIndexes();
        for (int i = 0; i < wrapper.GetObjectCount(); ++i) {
			(*targets)[i] = wrapper.GetTarget(i);
        }
    }
}

const FILE* logger = LOGHDR(stderr);

int main(int argc, char** argv) {
	{	// Logger initialization...
		char path[80] = {0};
		time_t rawtime = time(NULL);
		struct tm timeinfo = *localtime(&rawtime);
		strftime(path, sizeof(path), "mll_%y%m%d_%H%M%S.log", &timeinfo);
		LOGHDR(fopen(path, "a"));
	}
    try {
		typedef TCLAP::ValueArg<string> StringArg;
		typedef TCLAP::UnlabeledValueArg<string> UnlabeledStringArg;

        TCLAP::CmdLine cmd("Command description message", ' ', "0.1");

		UnlabeledStringArg commandTypeArg(
			"command", "Type of command", false, "", "string", cmd);

		StringArg classifierArg(
			"c", "classifier", "Name of classifier", false, "", "string", cmd);
		StringArg fullDataArg(
			"", "data", "File with full data", false, "", "string", cmd);
		//StringArg testDataArg(
		//	"", "trainData", "File with train data", false, "", "string", cmd);
		//StringArg trainDataArg(
		//	"", "testData", "File with test data", false, "", "string", cmd);
		StringArg testIndexesArg(
			"", "testIndexes", "File with test indexes", false, "", "string", cmd);
		StringArg trainIndexesArg(
			"", "trainIndexes", "File with train indexes", false, "", "string", cmd);
		//StringArg penaltiesArg(
		//	"", "penalties", "File with penalties", false, "", "string", cmd);
		
		StringArg testTargetOutputArg(
			"", "testTargetOutput", "File to write target of test set", 
			false, "", "string", cmd);
		StringArg testConfidencesOutputArg(
			"", "testConfidencesOutput", "File to write confidences of test set", 
			false, "", "string", cmd);
		//StringArg testObjectsWeightsOutputArg(
		//	"", "testFeatureWeightsOutput", "File to write objects weights of test set", 
		//	false, "", "string", cmd);

		StringArg trainTargetOutputArg(
			"", "trainTargetOutput", "File to write target of test set", 
			false, "", "string", cmd);
		StringArg trainConfidencesOutputArg(
			"", "trainConfidencesOutput", "File to write confidences of test set", 
			false, "", "string", cmd);
		//StringArg trainObjectsWeightsOutputArg(
		//	"", "trainFeatureWeightsOutput", "File to write objects weights of test set", 
		//	false, "", "string", cmd);

		//StringArg featureWeightsOutputArg(
		//	"", "featureWeightsOutput", "File to write feature weights", 
		//	false, "", "string", cmd);

		// Parsing command line...
		cmd.parse(argc, argv);

		{	// Logging command line args...
			std::list<TCLAP::Arg*>& argList = cmd.getArgList();
			for (std::list<TCLAP::Arg*>::iterator it = argList.begin()
				; it != argList.end(); ++it) {
				if (!(*it)->isSet()) continue;
				StringArg* strArg = dynamic_cast<StringArg*>(*it);
				if (strArg) { 
					LOGD("%s: %s", LOGSTR((*it)->getName()), LOGSTR(strArg->getValue()));
					continue;
				}
				UnlabeledStringArg* ulArg = dynamic_cast<UnlabeledStringArg*>(*it);
				if (ulArg) {
					LOGD("%s: %s", LOGSTR((*it)->getName()), LOGSTR(ulArg->getValue()));
					continue;
				}
			}
		}

		if (commandTypeArg.getValue() == "classify") {

			LOGI("Classification mode...");

			sh_ptr<DataSet> dataSet = LoadDataSet(fullDataArg.getValue());
			sh_ptr<IClassifier> classifier = CreateClassifier(classifierArg.getValue());

			DataSetWrapper testSet(dataSet.get());
			DataSetWrapper trainSet(dataSet.get());

			{
				std::vector<int> indexes;
				indexes.reserve(dataSet->GetObjectCount());
				// Loading train indexes...
				LoadVector(trainIndexesArg.getValue(), &indexes);
				trainSet.SetObjectIndexes(indexes.begin(), indexes.end());
				// Loading test indexes...
                LoadVector(testIndexesArg.getValue(), &indexes);
                testSet.SetObjectIndexes(indexes.begin(), indexes.end());
			}

			LOGI("Learning...");
			classifier->Learn(&trainSet);

			{
				std::vector<int> targets;
				std::vector<float> confidence;
				targets.reserve(dataSet->GetObjectCount());

				LOGI("Classifing train data set...");
				Classify(*classifier, &trainSet, &targets, &confidence);
				if (trainConfidencesOutputArg.isSet()) {
					WriteMatrix(trainConfidencesOutputArg.getValue()
								, confidence
								, trainSet.GetClassCount());
				}
				WriteVector(trainTargetOutputArg.getValue(), targets);

				LOGI("Classifing test data set...");
				Classify(*classifier, &testSet, &targets, &confidence);
                if (testConfidencesOutputArg.isSet()) {
                    WriteMatrix(testConfidencesOutputArg.getValue()
								, confidence
								, trainSet.GetClassCount());
                }
				WriteVector(testTargetOutputArg.getValue(), targets);
			}	
		}
		else {
			ListClassifiers();
            ListTesters();
		}
    }
    catch (TCLAP::ArgException &ex) {
		LOGE("Error: %s for arg %s", LOGSTR(ex.error()), LOGSTR(ex.argId()));
		exit(EXIT_FAILURE);
    }
	catch (const std::exception& ex) {
		LOGE("Error occurred: %s", (ex.what()));
        exit(EXIT_FAILURE);
    }
	catch (...) {
		LOGE("Unhandled error occured");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
