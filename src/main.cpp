#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>

#include <tclap/CmdLine.h>

#include "classifier.h"
#include "cross_validation.h"
#include "dataset.h"
#include "factories.h"
#include "tester.h"

using std::cout;
using std::cerr;
using std::endl;
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

sh_ptr<IDataSet> LoadDataSet(const string& dataFileName) {
    DataSet *dataSet = new DataSet();
    if (!dataSet->Load(dataFileName)) {
        throw std::logic_error("Could not load dataset");
    }
    cout << "Dataset '" << dataSet->GetName() << "' loaded: "
         << dataSet->GetFeatureCount() << " features, "
         << dataSet->GetObjectCount() << " objects, "
         << dataSet->GetClassCount() << " classes." << endl;
    cout << endl;
    return sh_ptr<IDataSet>(dataSet);
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

vector<int> readIndexes(const string& filename) {
	ifstream fin(filename.c_str());
	if (!fin.is_open()) {
		throw std::invalid_argument("File doesn't exists!");
	}
	vector<int> res;
	int index;
	while (fin >> index) {
		res.push_back(index);
	}
	fin.close();
	return res;
}

sh_ptr<IDataSet> GetData(const string& filename) {
	return LoadDataSet(filename);
}

sh_ptr<IDataSet> GetData(const IDataSet&, const vector<int>&) {
	// Empty function
	return sh_ptr<IDataSet>(new DataSet());
}

void OutputTargets(const string& filename, const IDataSet&) {
}

void OutputConfidences(const string& filename, const IDataSet&) {
}

void OutputWeights(const string& filename, const IDataSet&) {
}

void LoadPenalties(IDataSet* dataSet, const string& filename) {
}


int main(int argc, char** argv) {
    try {
		typedef TCLAP::ValueArg<string> StringArg;
        TCLAP::CmdLine cmd("Command description message", ' ', "0.1");

		TCLAP::UnlabeledValueArg<string> commandTypeArg(
			"command", "Type of command", true, "", "string", cmd);
		StringArg classifierArg(
			"c", "classifier", "Name of classifier", false, "", "string", cmd);
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

		cmd.parse(argc, argv);

		if (!classifierArg.isSet()) {
			throw TCLAP::ArgException("Classifier requeriment!");
		}

		if (!testTargetOutputArg.isSet()) {
			throw TCLAP::ArgException("File to test targets requeriment!");
		}
		
		sh_ptr<IDataSet> trainData;
		sh_ptr<IDataSet> testData;
		if (fullDataArg.isSet()) {
			if (testDataArg.isSet() || trainDataArg.isSet() || 
				!testIndexesArg.isSet() || !trainIndexesArg.isSet()) {
				throw TCLAP::ArgException("Error!");
			}
			sh_ptr<IDataSet> fullData = GetData(fullDataArg.getValue());
			trainData = GetData(*(fullData.get()), readIndexes(trainIndexesArg.getValue()));
			testData = GetData(*(fullData.get()), readIndexes(testIndexesArg.getValue()));
		}
		else {
			if (!testDataArg.isSet() || !trainDataArg.isSet() || 
				testIndexesArg.isSet() || trainIndexesArg.isSet()) {
				throw TCLAP::ArgException("Error!");
			}
			trainData = GetData(trainDataArg.getValue());
			testData = GetData(testDataArg.getValue());
		}

		sh_ptr<IClassifier> classifier = CreateClassifier(classifierArg.getValue());

		if (penaltiesArg.isSet()) {
			LoadPenalties(trainData.get(), penaltiesArg.getValue());
		}

		classifier->Learn(trainData.get());

		if (trainTargetOutputArg.isSet() || 
			trainConfidencesOutputArg.isSet() ||
			trainObjectsWeightsOutputArg.isSet()) {
			classifier->Classify(trainData.get());
			if (trainTargetOutputArg.isSet()) {
				OutputTargets(trainTargetOutputArg.getValue(), *(trainData.get()));
			}
			if (trainConfidencesOutputArg.isSet()) {
				OutputConfidences(trainConfidencesOutputArg.getValue(), *(trainData.get()));
			}
			if (trainObjectsWeightsOutputArg.isSet()) {
				OutputWeights(trainObjectsWeightsOutputArg.getValue(), *(trainData.get()));
			}
		}

		classifier->Classify(testData.get());
		if (testTargetOutputArg.isSet()) {
			OutputTargets(testTargetOutputArg.getValue(), *(testData.get()));
		}
		if (testConfidencesOutputArg.isSet()) {
			OutputConfidences(testConfidencesOutputArg.getValue(), *(testData.get()));
		}
		if (testObjectsWeightsOutputArg.isSet()) {
			OutputWeights(testObjectsWeightsOutputArg.getValue(), *(testData.get()));
		}

		
		//TCLAP::ValueArg<std::string> nameArg("n", "name", "Name to print", true, "homer", "string");

  //      cmd.add( nameArg );

  //      TCLAP::SwitchArg reverseSwitch("r","reverse","Print name backwards", cmd, false);

  //      cmd.parse( argc, argv );

  //      std::string name = nameArg.getValue();
  //      bool reverseName = reverseSwitch.getValue();

  //      if (reverseName) {
  //          std::reverse(name.begin(),name.end());
  //          std::cout << "My name (spelled backwards) is: " << name << std::endl;
  //      }
  //      else {
  //          std::cout << "My name is: " << name << std::endl;
  //      }

  //      cout << "Welcome to the Machine Learning Library!" << endl << endl;

  //      ListClassifiers();
  //      RegisterCVTesters();
  //      ListTesters();
  //      sh_ptr<DataSet> dataSet = LoadDataSet("../data/iris.arff");
  //      sh_ptr<IClassifier> classifier = CreateClassifier("NaiveBayes");
  //      sh_ptr<ITester> tester = CreateTester("Random");

  //      cout << "Testing..." << endl;
  //      double error = tester->Test(*classifier, dataSet.get());
  //      cout << "Errors: " << error << endl;

  //      return EXIT_SUCCESS;
    }
    catch (TCLAP::ArgException &e)
    { 
        cerr << "error: " << e.error() << " for arg " << e.argId() << endl; 
    }
	catch (const std::exception& ex) {
		cerr << "Exception occurred: " << ex.what() << endl;
        return EXIT_FAILURE;
    }
}
