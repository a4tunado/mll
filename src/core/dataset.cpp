#include <algorithm>
#include <limits>
#include <fstream>
#include <math.h>

#include "dataset.h"
#include "util.h"

using std::string;
using std::vector;

namespace mll {

namespace {

class Tokenizer {
public:
    Tokenizer(const string& data)
        : data_(data),
          position_(0) {
        SkipWhiteSpaces();
    }

    const string& GetData() const {
        return data_;
    }

    int GetPosition() const {
        return position_;
    }

    void SetPosition(int position) {
        if (position >= 0) {
            position_ = position;
        }
    }

    const string& GetCurrentToken() const {
        return currentToken_;
    }

    template<typename T>
    T GetCurrent() {
        return mll::FromString<T>(currentToken_);
    }

    vector<char>& GetQuotes() {
        return quotes_;
    }

    const vector<char>& GetQuotes() const {
        return quotes_;
    }

    vector<char>& GetDelimeters() {
        return delimiters_;
    }

    const vector<char>& GetDelimeters() const {
        return delimiters_;
    }

    bool ReadNext() {
        SkipWhiteSpaces();
        if (position_ == data_.length()) {
            return false;
        }
        int length = 0;
        int skip;
        if (std::find(quotes_.begin(), quotes_.end(), data_[position_]) != quotes_.end()) {
            ++position_;
            while (position_ + length < static_cast<int>(data_.length()) && 
                   std::find(quotes_.begin(), quotes_.end(), data_[position_ + length]) == quotes_.end()) {
                ++length;
            }
            skip = length + 1;
        } else {
            while (position_ + length < static_cast<int>(data_.length()) && 
                   std::find(delimiters_.begin(), delimiters_.end(), data_[position_ + length]) == delimiters_.end() && 
                   !isspace(data_[position_ + length])) {
                ++length;
            }
            skip = length;
        }
        currentToken_ = data_.substr(position_, length);
        position_ += skip;
        SkipWhiteSpaces();
        if (position_ < static_cast<int>(data_.length()) && 
            std::find(delimiters_.begin(), delimiters_.end(), data_[position_]) != delimiters_.end())
        {
            ++position_;
        }
        return true;
    }


private:
    void SkipWhiteSpaces() {
        while (position_ < static_cast<int>(data_.length()) && isspace(data_[position_])) {
            ++position_;
        }
    }

    string data_;
    int position_;
    vector<char> delimiters_;
    vector<char> quotes_;
    string currentToken_;
};

} // namespace

DataSet::DataSet(const IDataSet& dataSet)
    : metaData_(dataSet.GetMetaData()),
      objectCount_(dataSet.GetObjectCount()),
      targets_(dataSet.GetObjectCount()),
      weights_(dataSet.GetObjectCount()),
      features_(dataSet.GetObjectCount(), vector<double>(dataSet.GetFeatureCount())) {
    for (int i = 0; i < objectCount_; ++i) {
        targets_.at(i) = dataSet.GetTarget(i);
        weights_.at(i) = dataSet.GetWeight(i);
        for (int j = 0; j < GetFeatureCount(); ++j) {
            features_.at(i).at(j) = dataSet.GetFeature(i, j);
        }
    }
}

bool DataSet::HasFeature(int objectIndex, int featureIndex) const {
    return !IsNaN(GetFeature(objectIndex, featureIndex));
}

double DataSet::GetFeature(int objectIndex, int featureIndex) const {
    bool canBeMissed = metaData_.GetFeatureInfo(featureIndex).CanBeMissed; // also checks range for featureIndex
    if (featureIndex < static_cast<int>(features_.at(objectIndex).size())) {
        return features_.at(objectIndex).at(featureIndex);
    } else {
        return canBeMissed ? NaN : 0;
    }
}

void DataSet::Resize(int objectCount, int featureCount) {
    metaData_.SetFeatureCount(featureCount);
    targets_.resize(objectCount);
    weights_.resize(objectCount);
    features_.resize(objectCount);
    objectCount_ = objectCount;
    for (int i = 0; i < objectCount; ++i) {
        features_[i].resize(featureCount);
    }
}

void DataSet::SetFeature(int objectIndex, int featureIndex, double feature) {
    features_.at(objectIndex).resize(metaData_.GetFeatureCount());
    features_.at(objectIndex).at(featureIndex) = feature;
}

void DataSet::Clear() {
    objectCount_ = 0;
    targets_.clear();
    weights_.clear();
    features_.clear();
}

bool DataSet::Load(const string& fileName, DataFileFormat format /*= UnknownFormat*/) {
    if (format == UnknownFormat) {
        if (fileName.length() >= 5 && fileName.substr(fileName.length() - 5) == ".arff") {
            format = Arff;
        } else {
            format = SvmLight;
        }
    }
    std::ifstream input(fileName.c_str());
    if (!input.is_open()) {
        return false;
    }
    Clear();
    if (format == Arff) {
        return LoadArff(input);
    } else {
        return LoadSvmLight(input);
    }
}

bool DataSet::LoadArff(std::istream& input) {
    bool dataAchieved = false;
    bool relationAchieved = false;
    int targetIndex = -1;
    while (!input.eof()) {
        string line;
        getline(input, line);
        Tokenizer tokenizer(line);
        tokenizer.GetQuotes().push_back('"');
        tokenizer.GetDelimeters().push_back(',');
        if (line[tokenizer.GetPosition()] == '%') {
            continue;
        }
        if (!tokenizer.ReadNext()) {
            continue;
        }
        if (!relationAchieved) {
            string tag = tokenizer.GetCurrentToken();
            ToLower(&tag);
            if (tag != "@relation") {
                return false;
            }
            if (!tokenizer.ReadNext()) {
                return false;
            }
            metaData_.SetName(tokenizer.GetCurrentToken());
            relationAchieved = true;
        } else if (!dataAchieved) {
            string tag = tokenizer.GetCurrentToken();
            ToLower(&tag);
            if (tag == "@data") {
                dataAchieved = true;
            } else if (tag == "@attribute") {
                if (!tokenizer.ReadNext()) {
                    return false;
                }
                string attributeName = tokenizer.GetCurrentToken();
                vector<string> nominalValues;
                FeatureType attributeType = UnknownType;
                if (line[tokenizer.GetPosition()] != '{') {
                    if (!tokenizer.ReadNext()) {
                        return false;
                    }
                    string type = tokenizer.GetCurrentToken();
                    ToLower(&type);
                    if (type == "numeric" || type == "real") {
                        attributeType = Numeric;
                    } else if (type == "string" || type == "date") {
                        attributeType = UnknownType;
                    } else {
                        return false;
                    }
                } else {
                    tokenizer.GetDelimeters().push_back('{');
                    tokenizer.GetDelimeters().push_back('}');
                    tokenizer.ReadNext();
                    attributeType = Nominal;
                    while (tokenizer.ReadNext()) {
                        nominalValues.push_back(tokenizer.GetCurrentToken());
                    }
                }
                if (attributeType == Nominal && attributeName == "class") {
                    metaData_.SetTargetInfo(FeatureInfo(attributeName, attributeType, false, nominalValues));
                    targetIndex = GetFeatureCount();
                } else {
                    metaData_.AddFeature(FeatureInfo(attributeName, attributeType, false, nominalValues));
                }
            } else {
                return false;
            }
        } else {
            if (targetIndex == -1) {
                return false;
            }
            int objectIndex = AddObject();
            tokenizer.SetPosition(0);
            int attributeIndex;
            for (attributeIndex = 0; tokenizer.ReadNext(); ++attributeIndex) {
                if (attributeIndex > GetFeatureCount() + 1) {
                    return false;
                }
                bool isTarget = attributeIndex == targetIndex;
                int featureIndex = attributeIndex > targetIndex ? attributeIndex - 1 : attributeIndex;
                FeatureInfo info = isTarget ? metaData_.GetTargetInfo() : metaData_.GetFeatureInfo(featureIndex);
                const string& current = tokenizer.GetCurrentToken();
                if (current == "?") {
                    if (isTarget) {
                        return false;
                    } else {
                        metaData_.SetFeatureCanBeMissed(attributeIndex, true);
                        SetFeature(objectIndex, featureIndex, NaN);
                    }
                } else {
                    switch (info.Type) {
                        case Numeric: {
                            SetFeature(objectIndex, featureIndex, tokenizer.GetCurrent<double>());
                            break;
                        }
                        case Nominal: {
                            int classValue = find(info.NominalValues.begin(), info.NominalValues.end(), tokenizer.GetCurrentToken()) - info.NominalValues.begin();
                            if (classValue >= static_cast<int>(info.NominalValues.size())) {
                                return false;
                            }
                            if (isTarget) {
                                SetTarget(objectIndex, classValue);
                            } else {
                                SetFeature(objectIndex, featureIndex, classValue);
                            }
                            break;
                        }
                        default: {
                            SetFeature(objectIndex, featureIndex, 0);
                            break;
                        }
                    }
                }
            }
            if (attributeIndex != GetFeatureCount() + 1) {
                return false;
            }
        }
    }
    return true;
}

bool DataSet::LoadSvmLight(std::istream& input) {
    return false; // not implemented yet
}

int DataSet::AddObject() {
    features_.push_back(vector<double>(GetFeatureCount()));
    targets_.push_back(0);
    weights_.push_back(1);
    return objectCount_++;
}

double DataSet::GetConfidence(int objectIndex, int target) const {
    if (static_cast<int>(confidences_.size()) > objectIndex &&
        confidences_.at(objectIndex).size() > 0)
    {
        return confidences_.at(objectIndex).at(target);
    } else {
        if (target == Refuse) {
            return 0;
        }
        int actualTarget = GetTarget(objectIndex);
        if (target == actualTarget) {
            return 1.0;
        } else if (actualTarget == Refuse) {
            return 1.0 / GetClassCount();
        } else {
            return 0;
        }
    }
}

void DataSet::SetConfidence(int objectIndex, int target, double confidence) {
    if (confidence >= 0) {
        if (static_cast<int>(confidences_.size()) <= objectIndex) {
            confidences_.resize(GetObjectCount());
        }
        if (confidences_.at(objectIndex).size() == 0) {
            confidences_.at(objectIndex).resize(GetClassCount());
        }
        confidences_.at(objectIndex).at(target) = confidence;
    }
}

} // namespace mll
