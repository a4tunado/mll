#ifndef TESTER_H_
#define TESTER_H_

#include "data.h"
#include "classifier.h"
#include "configurable.h"

namespace mll {

/*! Calculates weighted sum of errors in classification of objects in the test set
    by a classifier created by the classifier factory and trained with the train set
*/
double GetClassificationErrorSum(const IClassifier& classifier,
                                 IDataSet* trainSet,
                                 IDataSet* testSet);

/*! Calculates error of classification of the test set by a classifier
    created by the classifier factory and trained with the train set
*/
inline double GetClassificationError(const IClassifier& classifier,
                                     IDataSet* trainSet,
                                     IDataSet* testSet) {
    double weightSum = testSet->GetWeightSum();
    if (weightSum == 0) {
        return 0;
    }
    return GetClassificationErrorSum(classifier, trainSet, testSet) / weightSum;
}

//! Interface for classifiers testers
class ITester: public virtual IConfigurable {
public:
    /*! Calculate average error of classification by a classifier created by
        the classifier factory using the data set
    */
    virtual double Test(const IClassifier& classifier,
                        IDataSet* dataSet) const = 0;

    //! Get a copy of the tester
    virtual sh_ptr<ITester> Clone() const = 0;
};

//! Tester base class
template<typename TTester>
class Tester: public ITester, public Configurable<TTester> {
public:
    //! If the tester is registered in the tester factory.
    //! Must be initialized with REGISTER_TESTER macro in user's cpp file
    // static const bool Registered;

    //! Get a copy of the tester
    virtual sh_ptr<ITester> Clone() const {
        return sh_ptr<ITester>(
            new TTester(dynamic_cast<const TTester&>(*this)));
    }
};

} // namespace mll

#endif // TESTER_H_
