#include "metadata.h"

#include <limits>

using std::string;
using std::vector;

namespace mll {

MetaData::MetaData()
    : featureCount_(0),
      allowRefuse_(false) {
}

MetaData::MetaData(const IMetaData& metaData)
    : name_(metaData.GetName()),
      allowRefuse_(metaData.AllowRefuse()),
      featureCount_(metaData.GetFeatureCount()),
      featureNames_(metaData.GetFeatureCount()),
      featureCanBeMissed_(metaData.GetFeatureCount()),
      featureTypes_(metaData.GetFeatureCount()),
      featureNominalValues_(metaData.GetFeatureCount()),
      targetName_(metaData.GetTargetInfo().Name),
      targetNominalValues_(metaData.GetTargetInfo().NominalValues),
      penalties_(metaData.GetClassCount(), vector<double>(metaData.GetClassCount() + 1)) {
    for (int i = 0; i < featureCount_; ++i) {
        FeatureInfo info = metaData.GetFeatureInfo(i);
        featureNames_.at(i) = info.Name;
        featureTypes_.at(i) = info.Type;
        featureCanBeMissed_.at(i) =info.CanBeMissed;
        featureNominalValues_.at(i) = info.NominalValues;
    }
    for (int i = 0; i < GetClassCount(); ++i) {
        for (int j = 0; j < GetClassCount(); ++j) {
            penalties_.at(i).at(j) = metaData.GetPenalty(i, j);
        }
        penalties_.at(i).back() = metaData.GetPenalty(i, Refuse);
    }
}

int MetaData::GetFeatureCount() const {
    return featureCount_;
}

FeatureInfo MetaData::GetFeatureInfo(int featureIndex) const {
    return FeatureInfo(
        featureNames_.at(featureIndex),
        featureTypes_.at(featureIndex),
        featureCanBeMissed_.at(featureIndex),
        featureNominalValues_.at(featureIndex));
}

FeatureInfo MetaData::GetTargetInfo() const {
    return FeatureInfo(targetName_, Nominal, allowRefuse_, targetNominalValues_);
}

double MetaData::GetPenalty(int actualClass, int predictedClass) const {
    if (predictedClass == Refuse) {
        if (AllowRefuse()) {
            return penalties_.at(actualClass).back();
        } else {
            return std::numeric_limits<double>::infinity();
        }
    } else {
        return penalties_.at(actualClass).at(predictedClass);
    }
}

void MetaData::Clear() {
    featureCount_ = 0;
    targetName_.clear();
    featureNames_.clear();
    allowRefuse_ = false;
    featureCanBeMissed_.clear();
    featureTypes_.clear();
    targetNominalValues_.clear();
    featureNominalValues_.clear();
    penalties_.clear();
}

void MetaData::SetFeatureCount(int count) {
    featureNames_.resize(count);
    featureCanBeMissed_.resize(count);
    featureTypes_.resize(count);
    featureNominalValues_.resize(count);
    featureCount_ = count;
}

void MetaData::SetPenalty(int actualClass, int predictedClass, double penalty) {
    if (predictedClass == Refuse) {
        penalties_.at(actualClass).back() = penalty;
    } else {
        penalties_.at(actualClass).at(predictedClass) = penalty;
    }
}

void MetaData::AddFeature(FeatureInfo featureInfo) {
    featureNames_.push_back(featureInfo.Name);
    featureCanBeMissed_.push_back(featureInfo.CanBeMissed);
    featureTypes_.push_back(featureInfo.Type);
    featureNominalValues_.push_back(featureInfo.NominalValues);
    ++featureCount_;
}

void MetaData::SetFeatureName(int featureIndex, const string& name) {
    featureNames_.at(featureIndex) = name;
}

void MetaData::SetFeatureCanBeMissed(int featureIndex, bool canBeMissed) {
    featureCanBeMissed_.at(featureIndex) = canBeMissed;
}

void MetaData::SetFeatureType(int featureIndex, FeatureType type) {
    featureTypes_.at(featureIndex) = type;
}

void MetaData::SetFeatureNominalValues(int featureIndex, const vector<string>& nominalValues) {
    featureNominalValues_.at(featureIndex) = nominalValues;
}

void MetaData::SetTargetName(const string& name) {
    targetName_ = name;
}

void MetaData::SetAllowRefuse(bool allowRefuse) {
    allowRefuse_ = allowRefuse;
}

void MetaData::SetTargetNominalValues(const vector<string>& nominalValues) {
    targetNominalValues_ = nominalValues;
    penalties_.assign(GetClassCount(), vector<double>(GetClassCount() + 1, 1.0));
    for (int i = 0; i < GetClassCount(); ++i) {
        penalties_[i][i] = 0.0;
    }
}

} // namespace mll
