#ifndef DATASET_WRAPPER_H_
#define DATASET_WRAPPER_H_

#include <stdexcept>

#include "data.h"
#include "metadata_wrapper.h"

namespace mll {

//! Wrapper for dataset. Implements IDataSet.
/*! Can change some data properties (such as weights, targets, objects and 
    features set and order) while the original data keeps constant.
*/
class DataSetWrapper: public IDataSet {
public:
    //! Default initialization with the original data
    DataSetWrapper(const IDataSet* dataSet)
        : dataSet_(dataSet) {
        if (dataSet == NULL) {
            throw std::logic_error("DataSet cannot be null");
        }
    }

    //! Restores all properties to their original values
    void Reset() {
        metaData_.reset();
        featureIndexes_ = sh_ptr< std::vector<int> >();
        ResetObjectIndexes();
        ResetWeights();
        ResetTargets();
    }

    void ResetMetaData() {
        metaData_.reset();
        if (featureIndexes_.get() != NULL) {
            metaData_->SetFeatureIndexes(featureIndexes_);
        }
    }

    void ResetFeatureIndexes() {
        if (metaData_.get() != NULL) {
            metaData_->Reset();
        }
        featureIndexes_ = sh_ptr< std::vector<int> >();
    }

    void ResetObjectIndexes() {
        objectIndexes_ = sh_ptr< std::vector<int> >();
    }

    void ResetWeights() {
        weights_.reset();
    }

    void ResetTargets() {
        targets_.reset();
    }

    //! Gets metadata about the dataset
    virtual const IMetaData& GetMetaData() const {
        if (metaData_.get() != NULL) {
            return *metaData_;
        } else {
            return dataSet_->GetMetaData();
        }
    }

    //! Get number of objects in the dataset
    virtual int GetObjectCount() const {
        if (objectIndexes_.get() != NULL) {
            return objectIndexes_->size();
        } else {
            return dataSet_->GetObjectCount();
        }
    }

    //! Returns false if the feature value for the object is missed
    virtual bool HasFeature(int objectIndex, int featureIndex) const {
        return dataSet_->HasFeature(
            GetActualObjectIndex(objectIndex),
            GetActualFeatureIndex(featureIndex));
    }

    //! Gets the object's value of the feature
    virtual double GetFeature(int objectIndex, int featureIndex) const {
        return dataSet_->GetFeature(
            GetActualObjectIndex(objectIndex),
            GetActualFeatureIndex(featureIndex));
    }

    //! Gets the object's value of the target feature
    virtual int GetTarget(int objectIndex) const {
        if (targets_.get() != NULL) {
            return targets_->at(GetActualObjectIndex(objectIndex));
        } else {
            return dataSet_->GetTarget(GetActualObjectIndex(objectIndex));
        }
    }

    //! Gets the object's weight
    virtual double GetWeight(int objectIndex) const {
        if (weights_.get() != NULL) {
            return weights_->at(GetActualObjectIndex(objectIndex));
        } else {
            return dataSet_->GetWeight(GetActualObjectIndex(objectIndex));
        }
    }

    //! Sets metadata
    void SetMetaData(const IMetaData* metaData);

    //! Sets the object's value of the target feature
    virtual void SetTarget(int objectIndex, int target) {
        if (target >= 0 && target < GetClassCount() || target == Refuse) {
            CreateTargets();
            targets_->at(GetActualObjectIndex(objectIndex)) = target;
        }
    }

    //! Sets the object's weight
    virtual void SetWeight(int objectIndex, double weight) {
        if (weight >= 0) {
            CreateWeights();
            weights_->at(GetActualObjectIndex(objectIndex)) = weight;
        }
    }

    //! Swaps two objects
    virtual void SwapObjects(int objectIndex1, int objectIndex2) {
        CreateObjectIndexes();
        std::swap(objectIndexes_->at(objectIndex1), objectIndexes_->at(objectIndex2));
    }

    //! Sets subset (list) of object indices. Useful for testing.
    template<typename TIter>
    void SetObjectIndexes(TIter first, TIter last) {
        SetObjectIndexes(sh_ptr< std::vector<int> >(new std::vector<int>(first, last)));
    }

    //! Sets subset (list) of feature indices. Useful for feature selection.
    template<typename TIter>
    void SetFeatureIndexes(TIter first, TIter last) {
        SetFeatureIndexes(sh_ptr< std::vector<int> >(new std::vector<int>(first, last)));
    }
    
private:
    //! Gets index of the object in the original data by its index in the wrapper
    int GetActualObjectIndex(int objectIndex) const {
        if (objectIndexes_.get() != NULL) {
            return objectIndexes_->at(objectIndex);
        } else {
            return objectIndex;
        }
    }

    //! Gets index of the feature in the original data by its index in the wrapper.
    int GetActualFeatureIndex(int featureIndex) const {
        if (featureIndexes_.get() != NULL) {
            return featureIndexes_->at(featureIndex);
        } else {
            return featureIndex;
        }
    }

    //! Sets subset (list) of object indices
    void SetObjectIndexes(sh_ptr< std::vector<int> > objectIndexes);
    //! Sets subset (list) of feature indices
    void SetFeatureIndexes(sh_ptr< std::vector<int> > featureIndexes);

    //! Creates vector of custom targets
    void CreateTargets();
    //! Creates vector of custom weights
    void CreateWeights();
    //! Creates vector of default object indices
    void CreateObjectIndexes();
    //! Creates metadata wrapper
    void CreateMetaData();

    //! Original data
    const IDataSet* dataSet_;

    //! Custom metadata wrapper
    std::auto_ptr<MetaDataWrapper> metaData_;
    //! Custom feature indices
    sh_ptr< std::vector<int> > featureIndexes_;
    //! Custom object indices
    sh_ptr< std::vector<int> > objectIndexes_;
    //! Custom object weights
    std::auto_ptr< std::vector<double> > weights_;
    //! Custom object targets
    std::auto_ptr< std::vector<int> > targets_;
};

} // namespace mll

#endif // DATASET_WRAPPER_H_
