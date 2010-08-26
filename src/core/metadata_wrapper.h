#ifndef METADATA_WRAPPER_H_
#define METADATA_WRAPPER_H_

#include <memory>
#include <stdexcept>

#include "data.h"
#include "sh_ptr.h"

namespace mll {

//! Wrapper for metadata. Implements IMetaData.
/*! Can change feature set and order while the original metadata keeps constant.
*/
class MetaDataWrapper: public IMetaData {
public:
    //! Default initialization with the original metadata
    MetaDataWrapper(const IMetaData* metaData)
        : metaData_(metaData) {
        if (metaData == NULL) {
            throw std::logic_error("MetaData cannot be null");
        }
    }

    //! Restores all properties to their original values
    void Reset() {
        featureIndexes_ = sh_ptr< std::vector<int> >();
    }

    //! Data name
    virtual const std::string& GetName() const {
        return metaData_->GetName();
    }

    //! Number of features in data
    virtual int GetFeatureCount() const {
        if (featureIndexes_.get() != NULL) {
            return featureIndexes_->size();
        } else {
            return metaData_->GetFeatureCount();
        }
    }

    //! Gets the feature's metadata
    virtual FeatureInfo GetFeatureInfo(int featureIndex) const {
        return metaData_->GetFeatureInfo(GetActualFeatureIndex(featureIndex));
    }

    //! Gets the target feature's metadata
    virtual FeatureInfo GetTargetInfo() const {
        return metaData_->GetTargetInfo();
    }

    //! Gets the value of loss function for predicted and actual class labels
    virtual double GetPenalty(int actualClass, int predictedClass) const {
        return metaData_->GetPenalty(actualClass, predictedClass); // TODO: custom penalties
    }

    //! Sets subset (list) of feature indices. Useful for feature selection.
    template<typename TIter>
    void SetFeatureIndexes(TIter first, TIter last) {
        SetFeatureIndexes(sh_ptr< std::vector<int> >(new std::vector<int>(first, last)));
    }

private:
    friend class DataSetWrapper;

    //! Gets index of the feature in the original data by its index in the wrapper.
    int GetActualFeatureIndex(int featureIndex) const {
        if (featureIndexes_.get() != NULL) {
            return featureIndexes_->at(featureIndex);
        } else {
            return featureIndex;
        }
    }

    //! Sets subset (list) of feature indices
    void SetFeatureIndexes(sh_ptr< std::vector<int> > indexes);

    //! Original metadata
    const IMetaData* metaData_;
    //! Custom feature indices
    sh_ptr< std::vector<int> > featureIndexes_;
};

} // namespace mll

#endif // METADATA_WRAPPER_H_
