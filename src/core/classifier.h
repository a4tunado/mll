#ifndef CLASSIFIER_H_
#define CLASSIFIER_H_

#include "data.h"
#include "dataset_wrapper.h"
#include "configurable.h"

namespace mll {

//! Interface for classifier
class IClassifier: public virtual IConfigurable {
public:
    //! Learn data
	virtual void Learn(IDataSet* data) = 0;

    //! Classify data
    //! Predicted class labels should be written to data targets
	virtual void Classify(IDataSet* data) const = 0;

	//! Calculate confidence matrix
	//! Dataset objects order shouldn't be changed
	virtual void Classify(/*const*/ IDataSet* data
							, std::vector<float>* confidence) const = 0;

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

	//! Classify data
    //! Predicted class labels should be written to data targets
    virtual void Classify(IDataSet* data) const = 0;

	//! Calculate confidence matrix
    //! Objects order shouldn't be changed
    virtual void Classify(/*const*/ IDataSet* data
							, std::vector<float>* confidence) const {
		DataSetWrapper wrapper(data);
        Classify(&wrapper);
        wrapper.ResetObjectIndexes();
		confidence->clear();
        confidence->resize(wrapper.GetObjectCount() * wrapper.GetClassCount(), 0.0);
        for (int i = 0; i < wrapper.GetObjectCount(); ++i) {
			if (wrapper.GetTarget(i) == Refuse) continue;
			(*confidence)[i * wrapper.GetClassCount() + wrapper.GetTarget(i)] = 1.0;
		}
	}

    //! Get a copy of the classifier
    virtual sh_ptr<IClassifier> Clone() const {
        return sh_ptr<IClassifier>(
            new TClassifier(dynamic_cast<const TClassifier&>(*this)));
    }
};

} // namespace mll

#endif // CLASSIFIER_H_
