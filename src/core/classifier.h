#ifndef CLASSIFIER_H_
#define CLASSIFIER_H_

#include "data.h"
#include "configurable.h"

namespace mll {

//! Interface for classifier
class IClassifier: public virtual IConfigurable {
public:
    //! Learn data
	virtual void Learn(IDataSet* data) = 0;

    //! Classify data
    //! Predicted class labels should be written to data targets.
	virtual void Classify(IDataSet* data) const = 0;

    //! Get a copy of the classifier
    virtual sh_ptr<IClassifier> Clone() const = 0;

    //! Destructor
    virtual ~IClassifier() {
    }
};

//! Classifier base class
template<typename TClassifier>
class Classifier: public IClassifier, public Configurable<TClassifier> {
public:
    //! If the classifier is registered in the classifier factory.
    //! Must be initialized with REGISTER_CLASSIFIER macro in user's cpp file
    // static const bool Registered;

    //! Get a copy of the classifier
    virtual sh_ptr<IClassifier> Clone() const {
        return sh_ptr<IClassifier>(
            new TClassifier(dynamic_cast<const TClassifier&>(*this)));
    }
};

} // namespace mll

#endif // CLASSIFIER_H_
