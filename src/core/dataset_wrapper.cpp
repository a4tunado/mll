#include "dataset_wrapper.h"

#include <stdexcept>

#include "util.h"

using std::vector;

namespace mll {

void DataSetWrapper::SetMetaData(const IMetaData* metaData) {
    metaData_.reset(new MetaDataWrapper(metaData));  
    if (featureIndexes_.get() != NULL) {
        metaData_->SetFeatureIndexes(featureIndexes_);
    }
}

void DataSetWrapper::SetObjectIndexes(sh_ptr< vector<int> > objectIndexes) {
    for (vector<int>::const_iterator it = objectIndexes->begin(); it != objectIndexes->end(); ++it) {
        if (*it < 0 || *it >= dataSet_->GetObjectCount()) {
            throw std::out_of_range("Indexes was out of range");
        }
    }
    objectIndexes_ = objectIndexes;
}

void DataSetWrapper::SetFeatureIndexes(sh_ptr< vector<int> > featureIndexes) {
    CreateMetaData();
    metaData_->SetFeatureIndexes(featureIndexes);
    featureIndexes_ = featureIndexes;
}

void DataSetWrapper::CreateTargets() {
    if (targets_.get() == NULL) {
        targets_.reset(new vector<int>(dataSet_->GetObjectCount()));
        for (int i = 0; i < static_cast<int>(targets_->size()); ++i) {
            targets_->at(i) = dataSet_->GetTarget(i);
        }
    }
}

void DataSetWrapper::CreateWeights() {
    if (weights_.get() == NULL) {
        weights_.reset(new vector<double>(dataSet_->GetObjectCount()));
        for (int i = 0; i < static_cast<int>(targets_->size()); ++i) {
            weights_->at(i) = dataSet_->GetWeight(i);
        }
    }
}

void DataSetWrapper::CreateObjectIndexes() {
    if (objectIndexes_.get() == NULL) {
        objectIndexes_.set(new vector<int>());
        InitIndexes(dataSet_->GetObjectCount(), objectIndexes_.get());
    }
}

void DataSetWrapper::CreateMetaData() {
    if (metaData_.get() == NULL) {
        metaData_.reset(new MetaDataWrapper(&dataSet_->GetMetaData()));
    }
}

} // namespace mll
