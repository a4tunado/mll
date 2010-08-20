#ifndef METADATA_H_
#define METADATA_H_

#include "data.h"

namespace mll {

//! Simple metadata about dataset. Implements IMetaData.
class MetaData: public IMetaData {
public:
    //! Default initialization
    MetaData();

    //! Copy-constructor
    MetaData(const IMetaData& metaData);

    //! Clears all data from metadata
    void Clear();

    //! Data name
    virtual const std::string& GetName() const {
        return name_;
    }

    //! Sets data name
    void SetName(const std::string& name) {
        name_ = name;
    }

    //! Number of features in data
    virtual int GetFeatureCount() const;
    //! Sets number of features in data
    void SetFeatureCount(int count);
    //! Gets the feature's metadata
    virtual FeatureInfo GetFeatureInfo(int featureIndex) const;

    //! Sets the feature's metadata
    void SetFeatureInfo(int featureIndex, FeatureInfo featureInfo) {
        SetFeatureName(featureIndex, featureInfo.Name);
        SetFeatureType(featureIndex, featureInfo.Type);
        SetFeatureCanBeMissed(featureIndex, featureInfo.CanBeMissed);
        SetFeatureNominalValues(featureIndex, featureInfo.NominalValues);
    }

    //! Gets the target feature's metadata
    virtual FeatureInfo GetTargetInfo() const;

    //! Sets the target feature's metadata
    void SetTargetInfo(FeatureInfo targetInfo) {
        SetTargetName(targetInfo.Name);
        SetAllowRefuse(targetInfo.CanBeMissed);
        SetTargetNominalValues(targetInfo.NominalValues);
    }

    //! Gets the value of loss function for predicted and actual class labels
    virtual double GetPenalty(int actualClass, int predictedClass) const;
    //! Sets the value of loss function for predicted and actual class labels
    void SetPenalty(int actualClass, int predictedClass, double penalty);

    //! Adds one feature to the end with no info set
    void AddFeature(FeatureInfo featureInfo);
    //! Sets the feature name
    void SetFeatureName(int featureIndex, const std::string& name);
    //! Sets if the feature can be missed
    void SetFeatureCanBeMissed(int featureIndex, bool canBeMissed);
    //! Sets the feature value type
    void SetFeatureType(int featureIndex, FeatureType type);
    //! Sets the names of all nominal values which the feature can have
    void SetFeatureNominalValues(int featureIndex, const std::vector<std::string>& nominalValues);
    //! Sets the target feature name
    void SetTargetName(const std::string& name);
    //! Sets if classification refusal is allowable
    void SetAllowRefuse(bool allowRefuse);
    //! Sets the names of all nominal values which the target feature can have
    void SetTargetNominalValues(const std::vector<std::string>& nominalValues);
    
private:
    std::string name_;                      //!< Data name
    int featureCount_;                      //!< Number of features
    bool allowRefuse_;                      //!< If classification refusal if allowable
    std::vector<std::string> featureNames_; //!< Names of features
    std::vector<bool> featureCanBeMissed_;  //!< If features values can be missed
    std::vector<FeatureType> featureTypes_; //!< Features values types
    //! Names of nominal values for all features
    std::vector< std::vector<std::string> > featureNominalValues_; 
    std::string targetName_;                        //!< Target feature name
    std::vector<std::string> targetNominalValues_;  //!< Names of target nominal values
    std::vector< std::vector<double> > penalties_;  //!< Penalty matrix
};

} // namespace mll

#endif // METADATA_H_
