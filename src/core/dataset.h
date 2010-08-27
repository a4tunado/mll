#ifndef DATASET_H_
#define DATASET_H_

#include "data.h"
#include "metadata.h"

namespace mll {

//! Format of file with data
enum DataFileFormat {
    UnknownFormat,
    Arff,
    SvmLight
};

//! Simple dataset. Implements IDataSet interface.
class DataSet : public IDataSet {	
public:
    //! Default initialization
	DataSet()
        : objectCount_(0) {
    }

    //! Copy-constructor
    DataSet(const IDataSet& dataSet);

    //! Gets metadata about the dataset
    virtual const IMetaData& GetMetaData() const {
        return metaData_;
    }

    //! Gets metadata about the dataset
    MetaData& GetMetaData() {
        return metaData_;
    }

    //! Get number of objects in the dataset
    virtual int GetObjectCount() const {
        return objectCount_;
    }

    //! Returns false if the feature value for the object is missed
    virtual bool HasFeature(int objectIndex, int featureIndex) const;

    //! Gets the object's value of the feature
    virtual double GetFeature(int objectIndex, int featureIndex) const;

    //! Sets the object's value of the feature
    void SetFeature(int objectIndex, int featureIndex, double feature);

    //! Gets the object's value of the target feature
    virtual int GetTarget(int objectIndex) const {
        return targets_.at(objectIndex);
    }

    //! Sets the object's value of the target feature
    virtual void SetTarget(int objectIndex, int target) {
        if (target >= 0 && target < GetClassCount()) {
            targets_.at(objectIndex) = target;
        }
    }

    //! Gets the object's weight
    virtual double GetWeight(int objectIndex) const {
        return weights_.at(objectIndex);
    }

    //! Sets the object's weight
    virtual void SetWeight(int objectIndex, double weight) {
        if (weight >= 0) {
            weights_.at(objectIndex) = weight;
        }
    }

    //! Returns true if the dataset has matrix of confidences
    virtual bool HasConfidences() const {
        return confidences_.size() > 0;
    }

    //! Gets the object classification confidence for the target
    virtual double GetConfidence(int objectIndex, int target) const;

    //! Sets the object classification confidence for the target
    virtual void SetConfidence(int objectIndex, int target, double confidence);

    //! Swaps two objects
    virtual void SwapObjects(int objectIndex1, int objectIndex2) {
        std::swap(features_.at(objectIndex1), features_.at(objectIndex2));
        std::swap(targets_.at(objectIndex1), targets_.at(objectIndex2));
        std::swap(weights_.at(objectIndex1), weights_.at(objectIndex2));
    }

	//! Clears all data from dataset
	void Clear();

    //! Resizes dataset
    void Resize(int objectCount, int featureCount);

    //! Adds one object to the end with no features set
    int AddObject();

	//! Loads data from file
    bool Load(const std::string& fileName, DataFileFormat format = UnknownFormat);

private:
    //! Loads data from ARFF file
    bool LoadArff(std::istream& input);
    //! Loads data from SVM-Light file
    bool LoadSvmLight(std::istream& input);

	MetaData metaData_;				                    //!< Metadata
    int objectCount_;                                   //!< Number of objects
    std::vector< std::vector<double> > features_;       //!< Features matrix
    std::vector<int> targets_;                          //!< Targets vector
    std::vector<double> weights_;                       //!< Weights vector
    std::vector< std::vector<double> > confidences_;    //!< Confidences matrix
};

} // namespace mll

#endif // DATASET_H_
