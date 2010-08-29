#include <iostream>
#include <stdexcept>
#include <string.h>

#include "classifier.h"
#include "cross_validation.h"
#include "dataset.h"
#include "factories.h"
#include "tester.h"
#include "logger.h"

using std::cout;
using std::endl;
using std::vector;
using std::string;

using namespace mll;

struct ProgramArg {
    const char* short_;
    const char* long_;
    const char* description_;
    const char* default_;
};

ProgramArg g_args[] = {
    { "-?", "--help", "Display this message", NULL }
    , { "-c", "--classifier"          , "Classifier name"									, NULL  }
    , { "-d", "--dataFile"            , "Received data file path"							, NULL  }
    , { "-l", "--learningIndexes"     , "Leraning indexes file path"						, NULL  }
    , { "-t", "--testIndexes"         , "Test indexes file path"							, NULL  }
    , { "-p", "--penalies"            , "Penalty matrix file path"							, NULL  }
    , { "-r", "--algProperties"       , "Classifier properties output file path"			, NULL  }
    , { "-T", "--targetOutput"        , "Target output file path"							, NULL  }
    , { "-C", "--confidenceOutput"    , "Confidence output file path"						, NULL  }
    , { "-F", "--featureWeightsOutput", "Feature weights output file path"					, NULL  }
    , { "-W", "--objectWeightsOutput" , "Object weights output file path"					, NULL  }
    , { "-L", "--logFile"             , "Log file path"										, NULL  }
	, { "-V", "--logLevel"            , "Log level mask: ERR=0x1 WRN=0x2 INF=0x4 DBG=0x8"	, "0xf" }
    , { NULL }
};

enum {
    ARG_HELP = 0
    , ARG_CLASSIFIER
    , ARG_DATAFILE
    , ARG_LEARNINGINDEXES
    , ARG_TESTINDEXES
    , ARG_PENALTIES
    , ARG_ALGPROPERTIES
    , ARG_TARGETOUTPUT
    , ARG_CONFIDENCEOUTPUT
    , ARG_FEATUREWEIGHTSOUTPUT
    , ARG_OBJECTWEIGHTSOUTPUT
	, ARG_LOGFILE
	, ARG_LOGLEVEL
};

const char** GetProgramArg(int argc, const char* argv[], const ProgramArg& arg) {
    const char** argv_it = argv;
    const char** argv_end = argv + argc;
    unsigned int short_len = strlen(arg.short_);
    unsigned int long_len = strlen(arg.long_);
    while (++argv_it != argv_end) {
        if (!strncmp(*argv_it, arg.short_, short_len)
            || !strncmp(*argv_it, arg.long_, long_len)) {
            break;
        }
    }    
    return argv_it == argv_end ? NULL : argv_it;
}

const char* GetProgramArgValue(int argc, const char* argv[], const ProgramArg& arg) {
    const char** argv_it = GetProgramArg(argc, argv, arg);
    if (argv_it++) {
        const char** argv_end = argv + argc;
        if (argv_it != argv_end && (*argv_it)[0] != '-') {
            return *argv_it;
        }
    }
    return arg.default_;
}

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

int main(int argc, const char* argv[]) {

	LOGHANDLER(stderr);

    try {
        cout << "Welcome to the Machine Learning Library!" << endl << endl;
        
		if (argc < 2 || GetProgramArg(argc, argv, g_args[ARG_HELP])) {
			// outputting usage list
            ProgramArg* it = g_args;
            while ((++it)->short_) {
                cout << it->short_ << " " << it->long_ << endl;
                cout << "\t" << it->description_ << endl;
            }
            cout << endl;
        }

		if (GetProgramArg(argc, argv, g_args[ARG_LOGFILE])) {
			// log file
			FILE* logFile = fopen(
				GetProgramArgValue(
					argc, argv, g_args[ARG_LOGFILE]), "w");
			LOGHANDLER(logFile);
		}

		{	// logging out command args
			ProgramArg* it = g_args;
            while ((++it)->short_) {
				if (GetProgramArg(argc, argv, *it)) {
					const char* value = 
						GetProgramArgValue(argc, argv, *it);
					if (value) LOGI("%-16s: %s", it->long_, value);					
					else LOGI("%s", it->long_);
				}
			}
		}

		if (GetProgramArg(argc, argv, g_args[ARG_LOGLEVEL])) {
			// log level
			int logLevel = atoi(
				GetProgramArgValue(
					argc, argv, g_args[ARG_LOGLEVEL]));
					LOGLEVEL(logLevel);
		}

        ListClassifiers();
        RegisterCVTesters();
        ListTesters();
        sh_ptr<DataSet> dataSet = LoadDataSet("../data/iris.arff");
        sh_ptr<IClassifier> classifier = CreateClassifier("NaiveBayes");
        sh_ptr<ITester> tester = CreateTester("Random");

        cout << "Testing..." << endl;
        double error = tester->Test(*classifier, dataSet.get());
        cout << "Errors: " << error << endl;

    } catch (const std::exception& ex) {
        std::cerr << "Exception occurred: " << ex.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
