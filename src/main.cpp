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
        throw std::logic_error("No classifier with this name registered");
    }
    cout << "Classifier: " << classifierName << endl;
    cout << "Parameters:" << endl;
    classifier->PrintParameters(true, true);
    cout << endl;
    return classifier;
}

sh_ptr<ITester> CreateTester(const string& testerName) {
    sh_ptr<ITester> tester = TesterFactory::Instance().Create(testerName);
    if (tester.get() == NULL) {
        throw std::logic_error("No tester with this name registered");
    }
    cout << "Tester: " << testerName << endl;
    cout << "Parameters:" << endl;
    tester->PrintParameters(true, true);
    cout << endl;
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
		LOGF("Can't open input file: %s", LOGSTR(fileName));
	}
	data->clear();
	typename std::istream_iterator<T> it(input), end;
	std::copy(it, end, std::back_inserter(*data));
	LOGD("%d entries loaded from %s", data->size(), LOGSTR(fileName));
}

template<typename T>
void WriteVector(const std::string& fileName, const typename std::vector<T> &data) {
	std::ofstream output(fileName.c_str());
	if (!output.is_open()) {
		LOGF("Can't open output file: %s", LOGSTR(fileName));
	}
	for (typename std::vector<T>::const_iterator it = data.begin()
		; it != data.end(); ++it) {
		output << *it << std::endl;
	}
	LOGD("%d entries written into %s", data.size(), LOGSTR(fileName));
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
			"command", "Type of command", true, "", "string", cmd);

		StringArg classifierArg(
			"c", "classifier", "Name of classifier", true, "", "string", cmd);
		StringArg fullDataArg(
			"", "data", "File with full data", false, "", "string", cmd);
		StringArg testDataArg(
			"", "trainData", "File with train data", false, "", "string", cmd);
		StringArg trainDataArg(
			"", "testData", "File with test data", false, "", "string", cmd);
		StringArg testIndexesArg(
			"", "testIndexes", "File with test indexes", false, "", "string", cmd);
		StringArg trainIndexesArg(
			"", "trainIndexes", "File with train indexes", false, "", "string", cmd);
		StringArg penaltiesArg(
			"", "penalties", "File with penalties", false, "", "string", cmd);
		
		StringArg testTargetOutputArg(
			"", "testTargetOutput", "File to write target of test set", 
			false, "", "string", cmd);
		StringArg testConfidencesOutputArg(
			"", "testConfidencesOutput", "File to write confidences of test set", 
			false, "", "string", cmd);
		StringArg testObjectsWeightsOutputArg(
			"", "testFeatureWeightsOutput", "File to write objects weights of test set", 
			false, "", "string", cmd);

		StringArg trainTargetOutputArg(
			"", "trainTargetOutput", "File to write target of test set", 
			false, "", "string", cmd);
		StringArg trainConfidencesOutputArg(
			"", "trainConfidencesOutput", "File to write confidences of test set", 
			false, "", "string", cmd);
		StringArg trainObjectsWeightsOutputArg(
			"", "trainFeatureWeightsOutput", "File to write objects weights of test set", 
			false, "", "string", cmd);

		StringArg featureWeightsOutputArg(
			"", "featureWeightsOutput", "File to write feature weights", 
			false, "", "string", cmd);

		// Parsing command line...
		cmd.parse(argc, argv);

		{	// Logging command line args...
			std::list<TCLAP::Arg*>& argList = cmd.getArgList();
			for (std::list<TCLAP::Arg*>::iterator it = argList.begin()
				; it != argList.end(); ++it)
			{
				if (!(*it)->isSet()) continue;
				{
					StringArg* arg = dynamic_cast<StringArg*>(*it);
					if (arg) { 
						LOGD("ARG: %s: %s", LOGSTR((*it)->getName()), LOGSTR(arg->getValue()));
						continue;
					}
				}
				{
					UnlabeledStringArg* arg = dynamic_cast<UnlabeledStringArg*>(*it);
					if (arg) {
						LOGD("ARG: %s: %s", LOGSTR((*it)->getName()), LOGSTR(arg->getValue()));
						continue;
					}
				}
			}
		}

		if (commandTypeArg.getValue() == "classify") {

			LOGI("Enterring classification mode...");

			sh_ptr<DataSet> dataSet = LoadDataSet(fullDataArg.getValue());
			sh_ptr<IClassifier> classifier = CreateClassifier(classifierArg.getValue());

			DataSetWrapper testSet(dataSet.get());
			DataSetWrapper trainSet(dataSet.get());

			{
				std::vector<int> indexes;
				// Loading test indexes...
				LoadVector(testIndexesArg.getValue(), &indexes);
				testSet.SetObjectIndexes(indexes.begin(), indexes.end());
				// Loading train indexes...
				LoadVector(trainIndexesArg.getValue(), &indexes);
				trainSet.SetObjectIndexes(indexes.begin(), indexes.end());
			}
			LOGI("Learning...");
			classifier->Learn(&trainSet);
			{
				std::vector<int> targets;
				{
					DataSetWrapper trainSetWrapper(&trainSet);
					for (int i = 0; i < trainSetWrapper.GetObjectCount(); ++i) {
						trainSetWrapper.SetTarget(i, Refuse);
					}
					LOGI("Classification...");
					classifier->Classify(&trainSetWrapper);				
					trainSetWrapper.ResetObjectIndexes();
					targets.resize(trainSetWrapper.GetObjectCount());
					for (int i = 0; i < trainSetWrapper.GetObjectCount(); ++i) {
						targets[i] = trainSetWrapper.GetTarget(i);
					}
					WriteVector(trainTargetOutputArg.getValue(), targets);
				}
				{
					DataSetWrapper testSetWrapper(&trainSet);
                    for (int i = 0; i < testSetWrapper.GetObjectCount(); ++i) {
                        testSetWrapper.SetTarget(i, Refuse);
                    }
                    LOGI("Testing...");
                    classifier->Classify(&testSetWrapper);             
                    testSetWrapper.ResetObjectIndexes();
                    targets.resize(testSetWrapper.GetObjectCount());
                    for (int i = 0; i < testSetWrapper.GetObjectCount(); ++i) {
                        targets[i] = testSetWrapper.GetTarget(i);
                    }
                    WriteVector(testTargetOutputArg.getValue(), targets);
				}				
			}
		}
		else {
			throw std::logic_error("Command is not supported");
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
