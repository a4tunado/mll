#include "decision_stump.h"

#include <limits>
#include <vector>

#define ALGSYNONIM ""
#define PASSWORD ""

using std::vector;

REGISTER_CLASSIFIER(mll::roizner::DecisionStump,
                    "DecisionStump",
                    "MRoizner",
                    "Decision-stump classifier");

//const bool mll::roizner::DecisionStump::REGISTERED = 
//	RegisterClassifier<mll::roizner::DecisionStump>("DecisionStump"
//													, "MRoizner"
//													, "Decision-stump classifier");

namespace mll {
namespace roizner {

//! Choose the best class label for the weight sums on the one side of a threshold.
//! Returns the overall penalty for the selected class label
double SelectClassLabel(const vector<double>& classWeightSums,
                        const IMetaData& metaData,
                        int* classLabel) {
    double minPenalty = std::numeric_limits<double>::max();
    for (int label = 0; label < metaData.GetClassCount(); ++label) {
        double penalty = 0;
        for (int label1 = 0; label1 < metaData.GetClassCount(); ++label1) {
            penalty += classWeightSums[label1] * metaData.GetPenalty(label1, label);
        }
        if (penalty < minPenalty) {
            minPenalty = penalty;
            *classLabel = label;
        }
    }
    return minPenalty;
}

void DecisionStump::Learn(IDataSet* data) {
    double minPenalty = std::numeric_limits<double>::max();
    // Iterating by the feature
    for (int featureIndex = 0; featureIndex < data->GetFeatureCount(); ++featureIndex) {
        vector<double> belowThresholdWeightSums(data->GetClassCount());
        vector<double> aboveThresholdWeightSums(data->GetClassCount());
        // Initializing weight sums
        for (int objectIndex = 0; objectIndex < data->GetObjectCount(); ++objectIndex) {
            aboveThresholdWeightSums[data->GetTarget(objectIndex)] += data->GetWeight(objectIndex);
        }
        // Sorting by the feature
        data->SortObjectsByFeature(featureIndex);
        // Choosing best threshold
        for (int objectIndex = 0; objectIndex < data->GetObjectCount(); ++objectIndex) {
            belowThresholdWeightSums[data->GetTarget(objectIndex)] += data->GetWeight(objectIndex);
            aboveThresholdWeightSums[data->GetTarget(objectIndex)] -= data->GetWeight(objectIndex);
            int belowThresholdClass, aboveThresholdClass;
            double penalty =
                SelectClassLabel(belowThresholdWeightSums, data->GetMetaData(), &belowThresholdClass) +
                SelectClassLabel(aboveThresholdWeightSums, data->GetMetaData(), &aboveThresholdClass);
            if (penalty < minPenalty) {
                minPenalty = penalty;
                separatingFeatureIndex_ = featureIndex;
                belowThresholdClass_ = belowThresholdClass;
                aboveThresholdClass_ = aboveThresholdClass;
                double feature = data->GetFeature(objectIndex, featureIndex);
                threshold_ = 
                    objectIndex + 1 < data->GetObjectCount()
                        ? (feature + data->GetFeature(objectIndex + 1, featureIndex)) / 2
                        : feature + 1.0;
            }
        }
    }
}

void DecisionStump::Classify(IDataSet* data) const {
    for (int i = 0; i < data->GetObjectCount(); ++i) {
        bool below = data->GetFeature(i, separatingFeatureIndex_) < threshold_;
        data->SetTarget(i, below ? belowThresholdClass_ : aboveThresholdClass_);
    }
}

} // namespace roizner
} // namespace mll
