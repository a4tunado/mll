#ifndef DATA_H_
#define DATA_H_

#include <algorithm>
#include <string>
#include <vector>

#include "sh_ptr.h"
#include "util.h"

namespace mll {

//! Classification refusal
const int Refuse = -1;

//! Feature value type
enum FeatureType {
    UnknownType,
    Numeric,
    Binary,
    Nominal
};

//! Metadata about feature
struct FeatureInfo {
    //! Feature's name
    const std::string& Name;
    //! Feature's type
    FeatureType Type;
    //! If feature value can be missed in data
    bool CanBeMissed;
    //! Names of all nominal values which the feature can have
    const std::vector<std::string>& NominalValues;

    FeatureInfo(
        const std::string& name, 
        FeatureType type, 
        bool canBeMissed, 
        const std::vector<std::string>& nominalValues);
};

//! Metadata about dataset
class IMetaData {
public:
    //! Data name
    virtual const std::string& GetName() const = 0;
    //! Number of features in data
    virtual int GetFeatureCount() const = 0;
    //! Gets the feature's metadata
    virtual FeatureInfo GetFeatureInfo(int featureIndex) const = 0;
    //! Gets the target feature's metadata
    virtual FeatureInfo GetTargetInfo() const = 0;
    //! Gets the value of loss function for predicted and actual class labels
    virtual double GetPenalty(int actualClass, int predictedClass) const = 0;

    //! Number of classes (nominal values of the target feature)
    int GetClassCount() const;
    //! If classification refusal is allowable
    bool AllowRefuse() const;

    //! Destructor
    virtual ~IMetaData() {
    }
};

//! Dataset interface
class IDataSet {
public:
    //! Gets metadata about the dataset
    virtual const IMetaData& GetMetaData() const = 0;
    //! Get number of objects in the dataset
    virtual int GetObjectCount() const = 0;
    //! Returns false if the feature value for the object is missed
    virtual bool HasFeature(int objectIndex, int featureIndex) const = 0;
    //! Gets the object's value of the feature
    virtual double GetFeature(int objectIndex, int featureIndex) const = 0;
    //! Gets the object's value of the target feature
    virtual int GetTarget(int objectIndex) const = 0;
    //! Gets the object's weight
    virtual double GetWeight(int objectIndex) const = 0;

    //! Gets data name
    const std::string& GetName() const;
    //! Number of features in data
    int GetFeatureCount() const;
    //! Number of classes (nominal values of the target feature)
    int GetClassCount() const;
    //! Calculates the sum of weights of all objects
    double GetWeightSum() const;

    //! Sets the object's value of the target feature
    virtual void SetTarget(int objectIndex, int target) = 0;
    //! Sets the object's weight
    virtual void SetWeight(int objectIndex, double weight) = 0;
    //virtual void SetFeature(int objectIndex, int featureIndex, double feature) = 0; - maybe later

    //! Normalizes weights of all objects so that their sum equals to 1.0
    virtual void NormalizeWeights();

    //! Swaps two objects
    virtual void SwapObjects(int objectIndex1, int objectIndex2) = 0;
    //! Randomly shuffles objects
    void ShuffleObjects();
    //! Reverses the order of all objects
    void Reverse();
    //! Sorts all objects by predicate
    template<typename TPredicate>
    void SortObjects(TPredicate predicate);
    //! Sorts all objects by the feature value (ascending or descending)
    void SortObjectsByFeature(int featureIndex, bool reverse = false);
    //! Sorts all objects by weight (ascending or descending)
    void SortObjectsByWeight(bool reverse = false);
    //! Sorts all objects by the target feature value (ascending or descending)
    void SortObjectsByTarget(bool reverse = false);

    //! Destructor
    virtual ~IDataSet() {
    }
};

} // namespace mll

#define DATA_INL_H_
#include "data-inl.h"
#undef DATA_INL_H_

#endif // DATA_H_
