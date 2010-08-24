#include <iostream>
#include <stdexcept>
#include <string>

#include "classifier.h"
#include "cross_validation.h"
#include "dataset.h"
#include "factories.h"
#include "tester.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;

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

sh_ptr<DataSet> LoadDataSet(const string& dataFileName) {
    sh_ptr<DataSet> dataSet(new DataSet());
    if (!dataSet->Load(dataFileName)) {
        throw std::logic_error("Could not load dataset");
    }
    cout << "Dataset '" << dataSet->GetName() << "' loaded: "
         << dataSet->GetFeatureCount() << " features, "
         << dataSet->GetObjectCount() << " objects, "
         << dataSet->GetClassCount() << " classes." << endl;
    cout << endl;
    return dataSet;
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

int main(int argc, char** argv) {
    try {
        cout << "Welcome to the Machine Learning Library!" << endl << endl;

        ListClassifiers();
        RegisterCVTesters();
        ListTesters();
        sh_ptr<DataSet> dataSet = LoadDataSet("../data/iris.arff");
        sh_ptr<IClassifier> classifier = CreateClassifier("NaiveBayes");
        sh_ptr<ITester> tester = CreateTester("Random");

        cout << "Testing..." << endl;
        double error = tester->Test(*classifier, dataSet.get());
        cout << "Errors: " << error << endl;

        return EXIT_SUCCESS;
    } catch (const std::exception& ex) {
        std::cerr << "Exception occurred: " << ex.what() << endl;
        return EXIT_FAILURE;
    }
}
