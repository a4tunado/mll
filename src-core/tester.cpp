#include "tester.h"

#include "dataset_wrapper.h"

namespace mll {

double GetClassificationErrorSum(const IClassifier& classifier,
                                 IDataSet* trainSet,
                                 IDataSet* testSet) {
    sh_ptr<IClassifier> classifierCopy = classifier.Clone();
    DataSetWrapper testSetWrapper(testSet);
    for (int i = 0; i < testSet->GetObjectCount(); ++i) {
		testSetWrapper.SetTarget(i, Refuse);		
    }
    classifierCopy->Learn(trainSet);
    classifierCopy->Classify(&testSetWrapper);
    testSetWrapper.ResetObjectIndexes();

    const IMetaData& metaData = testSet->GetMetaData();
    double error = 0;
    for (int i = 0; i < testSet->GetObjectCount(); ++i) {
        error += testSet->GetWeight(i) *
                 metaData.GetPenalty(testSet->GetTarget(i), testSetWrapper.GetTarget(i));
    }
    return error;
}

} // namespace mll
