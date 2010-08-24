#ifndef CROSS_VALIDATION_H_
#define CROSS_VALIDATION_H_

#include "tester.h"
#include "factories.h"

namespace mll {

void RegisterCVTesters();

//! Random cross-validation tester
class RandomTester: public Tester<RandomTester> {
	DECLARE_REGISTRATION();
public:
    //! Default initialization
    RandomTester()
        : testCount_(10),
          testPortion_(0.3) {
        AddParameter("t", testCount_, &RandomTester::GetTestCount, &RandomTester::SetTestCount,
                     "Number of tests");
        AddParameter("r", testPortion_, &RandomTester::GetTestPortion, &RandomTester::SetTestPortion,
                     "Portion of objects left in test set");
    }


    /*! Calculate average error of classification by a classifier created by
        the classifier factory using the data set
    */
    virtual double Test(const IClassifier& classifier,
                        IDataSet* dataSet) const;

    //! Number of tests
    int GetTestCount() const {
        return testCount_;
    }

    //! Sets number of tests
    void SetTestCount(int testCount) {
        if (testCount >= 1) {
            testCount_ = testCount;
        }
    }

    //! Portion of objects left in test set
    double GetTestPortion() const {
        return testPortion_;
    }

    //! Sets the portion of objects left in test set
    void SetTestPortion(double testPortion) {
        if (testPortion >= 0 && testPortion < 1) {
            testPortion_ = testPortion;
        }
    }

private:
    int testCount_;         //!< Number of tests
    double testPortion_;    //!< Portion of objects left in test set
};

//! q-fold cross-validation tester
class QFoldTester: public Tester<QFoldTester> {
	DECLARE_REGISTRATION();
public:
    //! Default initialization
    QFoldTester()
        : foldCount_(10) {
        AddParameter("q", foldCount_, &QFoldTester::GetFoldCount, &QFoldTester::SetFoldCount,
                     "Number of folds (parameter 'q')");
    }

    /*! Calculate average error of classification by a classifier created by
        the classifier factory using the data set
    */
    virtual double Test(const IClassifier& classifier,
                        IDataSet* dataSet) const;

    //! Number of folds (parameter 'q')
    int GetFoldCount() const {
        return foldCount_;
    }

    //! Sets the number of folds
    void SetFoldCount(int foldCount) {
        if (foldCount >= 2) {
            foldCount_ = foldCount;
        }
    }

private:
    int foldCount_;     //!< Number of folds
};

//! t*q-fold cross-validation tester
class TQFoldTester: public Tester<TQFoldTester> {
	DECLARE_REGISTRATION();
public:
    //! Default initialization
    TQFoldTester()
        : foldCount_(10),
          testCount_(10) {
        AddParameter("q", foldCount_, &TQFoldTester::GetFoldCount, &TQFoldTester::SetFoldCount,
                     "Number of folds (parameter 'q')");
        AddParameter("t", testCount_, &TQFoldTester::GetTestCount, &TQFoldTester::SetTestCount,
                     "Number of tests");
    }

    /*! Calculate average error of classification by a classifier created by
        the classifier factory using the data set
    */
    virtual double Test(const IClassifier& classifier,
                        IDataSet* dataSet) const;

    //! Number of folds (parameter 'q')
    int GetFoldCount() const {
        return foldCount_;
    }

    //! Sets the number of folds
    void SetFoldCount(int foldCount) {
        if (foldCount >= 2) {
            foldCount_ = foldCount;
        }
    }

    //! Number of tests
    int GetTestCount() const {
        return testCount_;
    }

    //! Sets number of tests
    void SetTestCount(int testCount) {
        if (testCount >= 1) {
            testCount_ = testCount;
        }
    }

private:
    int foldCount_;     //!< Number of folds
    int testCount_;     //!< Number of tests
};

//! Leave-one-out cross-validation tester
class LeaveOneOutTester: public Tester<LeaveOneOutTester> {
	DECLARE_REGISTRATION();
public:
    /*! Calculate average error of classification by a classifier created by
        the classifier factory using the data set
    */
    virtual double Test(const IClassifier& classifier,
                        IDataSet* dataSet) const;
};

} // namespace mll

#endif // CROSS_VALIDATION_H_
