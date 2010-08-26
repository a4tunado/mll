#ifndef ROIZNER_DECISION_STUMP_H_
#define ROIZNER_DECISION_STUMP_H_

#include "classifier.h"
#include "factories.h"

namespace mll {
namespace roizner {

//! Decision-stump classifier
class DecisionStump: public Classifier<DecisionStump> {
	DECLARE_REGISTRATION();
public:
    //! Learn data
    virtual void Learn(IDataSet* data);
    //! Classify data
    virtual void Classify(IDataSet* data) const;

private:
    int separatingFeatureIndex_;    //!< Index of separating feature
    double threshold_;              //!< The feature value threshold
    int belowThresholdClass_;       //!< Class label for object below threshold
    int aboveThresholdClass_;       //!< Class label for object above threshold
};

} // namespace roizner
} // namespace mll

#endif
