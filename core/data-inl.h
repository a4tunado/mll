#ifndef DATA_INL_H_
#error "Direct inclusion of this file is not allowed, include data.h"
#endif
#undef DATA_INL_H_

namespace mll {

inline FeatureInfo::FeatureInfo(const std::string& name, FeatureType type, bool canBeMissed, const std::vector<std::string>& nominalValues)
    : Name(name),
      Type(type),
      CanBeMissed(canBeMissed),
      NominalValues(nominalValues) {
}

inline int IMetaData::GetClassCount() const {
    return GetTargetInfo().NominalValues.size();
}

inline bool IMetaData::AllowRefuse() const {
    return GetTargetInfo().CanBeMissed;
}

inline const std::string& IDataSet::GetName() const {
    return GetMetaData().GetName();
}

inline int IDataSet::GetFeatureCount() const {
    return GetMetaData().GetFeatureCount();
}

inline int IDataSet::GetClassCount() const {
    return GetMetaData().GetClassCount();
}

inline double IDataSet::GetWeightSum() const {
    double weightSum = 0;
    for (int i = 0; i < GetObjectCount(); ++i) {
        weightSum += GetWeight(i);
    }
    return weightSum;
}

inline void IDataSet::NormalizeWeights() {
    double weightSum = 0;
    for (int i = 0; i < GetObjectCount(); ++i) {
        weightSum += GetWeight(i);
    }
    if (weightSum == 0) {
        return;
    }
    for (int i = 0; i < GetObjectCount(); ++i) {
        SetWeight(i, GetWeight(i) / weightSum);
    }
}

inline void IDataSet::ShuffleObjects() {
    std::vector<int> indexes;
    InitIndexes(GetObjectCount(), &indexes);
    std::random_shuffle(indexes.begin(), indexes.end());
    for (int i = 0; i < static_cast<int>(indexes.size()); ++i) {
        int index = indexes[i];
        while (index < i) {
            indexes[i] = index = indexes[index];
        }
        SwapObjects(i, index);
    }
}

inline void IDataSet::Reverse() {
    for (int i = 0, j = GetObjectCount() - 1; i < j; ++i, --j) {
        SwapObjects(i, j);
    }
}

template<typename TPredicate>
inline void IDataSet::SortObjects(TPredicate predicate) {
    std::vector<int> indexes;
    InitIndexes(GetObjectCount(), &indexes);
    std::sort(indexes.begin(), indexes.end(), predicate);
    for (int i = 0; i < static_cast<int>(indexes.size()); ++i) {
        int index = indexes[i];
        while (index < i) {
            indexes[i] = index = indexes[index];
        }
        SwapObjects(i, index);
    }
}

class FeatureComparator {
private:
    const IDataSet& dataSet_;
    int featureIndex_;
    bool reverse_;

public:
    FeatureComparator(const IDataSet& dataSet, int featureIndex, bool reverse)
        : dataSet_(dataSet),
          featureIndex_(featureIndex),
          reverse_(reverse) {
    }

    bool operator() (int objectIndex1, int objectIndex2) const {
        if (reverse_) {
            return dataSet_.GetFeature(objectIndex1, featureIndex_) >
                   dataSet_.GetFeature(objectIndex2, featureIndex_);
        } else {
            return dataSet_.GetFeature(objectIndex1, featureIndex_) <
                   dataSet_.GetFeature(objectIndex2, featureIndex_);
        }
    }
};

class WeightComparator {
private:
    const IDataSet& dataSet_;
    bool reverse_;

public:
    WeightComparator(const IDataSet& dataSet, bool reverse)
        : dataSet_(dataSet),
          reverse_(reverse) {
    }

    bool operator() (int objectIndex1, int objectIndex2) const {
        if (reverse_) {
            return dataSet_.GetWeight(objectIndex1) >
                   dataSet_.GetWeight(objectIndex2);
        } else {
            return dataSet_.GetWeight(objectIndex1) <
                   dataSet_.GetWeight(objectIndex2);
        }
    }
};

class TargetComparator {
private:
    const IDataSet& dataSet_;
    bool reverse_;

public:
    TargetComparator(const IDataSet& dataSet, bool reverse)
        : dataSet_(dataSet),
          reverse_(reverse) {
    }

    bool operator() (int objectIndex1, int objectIndex2) const {
        if (reverse_) {
            return dataSet_.GetTarget(objectIndex1) >
                   dataSet_.GetTarget(objectIndex2);
        } else {
            return dataSet_.GetTarget(objectIndex1) <
                   dataSet_.GetTarget(objectIndex2);
        }
    }
};

inline void IDataSet::SortObjectsByFeature(int featureIndex, bool reverse) {
    SortObjects(FeatureComparator(*this, featureIndex, reverse));
}

inline void IDataSet::SortObjectsByWeight(bool reverse) {
    SortObjects(WeightComparator(*this, reverse));
}

inline void IDataSet::SortObjectsByTarget(bool reverse) {
    SortObjects(TargetComparator(*this, reverse));
}

} // namespace mll
