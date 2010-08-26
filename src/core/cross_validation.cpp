#include "cross_validation.h"

#include "dataset_wrapper.h"

using std::vector;

REGISTER_TESTER(mll::RandomTester, "Random", "MLL", "Random CV");
REGISTER_TESTER(mll::TQFoldTester, "TQFold", "MLL", "t*q-fold CV");
REGISTER_TESTER(mll::QFoldTester, "QFold", "MLL", "q-fold CV");
REGISTER_TESTER(mll::LeaveOneOutTester, "LOO", "MLL", "Leave-one-out CV");

namespace mll {

double RandomTester::Test(const IClassifier& classifier, IDataSet* dataSet) const {
    int testLength = static_cast<int>(dataSet->GetObjectCount() * testPortion_ + 1);
    double errors = 0;
    double testedWeightSum = 0;
    vector<int> indexes;
    InitIndexes(dataSet->GetObjectCount(), &indexes);
    for (int i = 0; i < testCount_; ++i) {
        dataSet->ShuffleObjects();
        DataSetWrapper testSetWrapper(dataSet);
        testSetWrapper.SetObjectIndexes(indexes.begin(), indexes.begin() + testLength);
        DataSetWrapper trainSetWrapper(dataSet);
        trainSetWrapper.SetObjectIndexes(indexes.begin() + testLength, indexes.end());
        testedWeightSum += testSetWrapper.GetWeightSum();
        errors += GetClassificationErrorSum(classifier, &trainSetWrapper, &testSetWrapper);
    }
    return testedWeightSum == 0 ? 0.0 : errors / testedWeightSum;
}

double QFoldTester::Test(const IClassifier& classifier, IDataSet* dataSet) const {
    int foldLength = dataSet->GetObjectCount() / foldCount_;
    int remainedObjects = dataSet->GetObjectCount() - foldCount_ * foldLength;
    double errors = 0;
    double weightSum = dataSet->GetWeightSum();
    vector<int> indexes;
    InitIndexes(dataSet->GetObjectCount(), &indexes);
    int offset = 0;
    for (int i = 0; i < foldCount_; ++i) {
        int testLength = foldLength;
        if (remainedObjects > 0) {
            --remainedObjects;
            ++testLength;
        }
        if (offset > 0) {
            std::swap_ranges(
                indexes.begin(),
                indexes.begin() + testLength,
                indexes.begin() + offset);
        }
        offset += testLength;

        DataSetWrapper testSetWrapper(dataSet);
        testSetWrapper.SetObjectIndexes(indexes.begin(), indexes.begin() + testLength);
        DataSetWrapper trainSetWrapper(dataSet);
        trainSetWrapper.SetObjectIndexes(indexes.begin() + testLength, indexes.end());
        errors += GetClassificationErrorSum(classifier, &trainSetWrapper, &testSetWrapper);
    }
    return errors / weightSum;
}

double TQFoldTester::Test(const IClassifier& classifier, IDataSet* dataSet) const {
    double errors = 0;
    QFoldTester tester;
    tester.SetFoldCount(foldCount_);
    for (int i = 0; i < testCount_; ++i) {
        dataSet->ShuffleObjects();
        errors += tester.Test(classifier, dataSet) / testCount_;
    }
    return errors;
}

double LeaveOneOutTester::Test(const IClassifier& classifier, IDataSet* dataSet) const {
    double errors = 0;
    double weightSum = dataSet->GetWeightSum();
    vector<int> indexes;
    InitIndexes(dataSet->GetObjectCount(), &indexes);
    DataSetWrapper testSetWrapper(dataSet);
    testSetWrapper.SetObjectIndexes(indexes.begin(), indexes.begin() + 1);
    DataSetWrapper trainSetWrapper(dataSet);
    trainSetWrapper.SetObjectIndexes(indexes.begin() + 1, indexes.end());
    for (int i = 0; i < dataSet->GetObjectCount(); ++i) {
        if (i > 0) {
            dataSet->SwapObjects(0, i);
        }
        errors += GetClassificationErrorSum(classifier, &trainSetWrapper, &testSetWrapper);
    }
    return errors / weightSum;
}

void RegisterCVTesters() {
    // REGISTER_TESTER macros will do automatically all needed
}

} // namespace mll
