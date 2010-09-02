#include "discretizor.h"
#include "core/logger.h"

using namespace mll;

void Discretizor::Discretize(/*const*/ IDataSet* dataSet
							, int times
							, int quotient
							, const IQualifier& qualifier
							, Intervals* intervals) const {	
	
	DataSetWrapper testDataSet(dataSet);
	DataSetWrapper trainDataSet(dataSet);

	int partition = dataSet->GetObjectCount() / quotient;

	std::vector<int> indexes(dataSet->GetObjectCount());
	for (int i = 0; i < dataSet->GetObjectCount(); i++) {
		indexes[i] = i;
	}

	Intervals currentIntervals;

	intervals->clear();
	for (int i = 0; i < dataSet->GetFeatureCount(); i++) {
		intervals->push_back(std::vector<double>(1, DBL_MAX));
	}

	std::vector< std::pair< Intervals, double > > intervalErrors;

	int limit = std::min(
					dataSet->GetFeatureCount() > 1 ? 1 : 100
					, dataSet->GetObjectCount() / 2);
	
	for (int k = 0; k < limit * dataSet->GetFeatureCount(); k++) {

		double minErrorSumm = DBL_MAX;
		Intervals minErrorIntervals = *intervals;
		
		for (int featureIndex = 0
			; featureIndex < dataSet->GetFeatureCount()
			; featureIndex++) {

			currentIntervals = *intervals;

			{
				std::random_shuffle(indexes.begin(), indexes.end());

				trainDataSet.SetObjectIndexes(
					indexes.begin() + partition, indexes.end());
			
				Discretize(&trainDataSet
					, featureIndex
					, currentIntervals[featureIndex].size()
					, &currentIntervals[featureIndex]);
			}

			double errorSumm = 0.;

			for (int t = 0; t < times; t++) {
				std::random_shuffle(indexes.begin(), indexes.end());
				
				testDataSet.SetObjectIndexes(
					indexes.begin(), indexes.begin() + partition);
				
				trainDataSet.SetObjectIndexes(
					indexes.begin() + partition, indexes.end());

				errorSumm += qualifier(&trainDataSet
										, &testDataSet
										, currentIntervals);
			}

			if (errorSumm < minErrorSumm) {
				minErrorSumm = errorSumm;
				minErrorIntervals = currentIntervals;
			}
		}

		*intervals = minErrorIntervals;

		intervalErrors.push_back(
			std::make_pair(minErrorIntervals, minErrorSumm / times));

		LOGI("Min Err: [%02d] [%02d] %f",  k, intervals->front().size(), intervalErrors.back().second);
	}

	{
		int iteration = -1;
		double minError = DBL_MAX;
		for (int k = 0; k < intervalErrors.size(); k++) {
			if (intervalErrors[k].second < minError) {
				iteration = k;
				minError = intervalErrors[k].second;
				*intervals = intervalErrors[k].first;
			}		
		}
		LOGI("Res Err: [%02d] %f", iteration, minError);
	}
}

void EwdDiscretizor::Discretize(
					/* const */ IDataSet* dataSet
					, int featureIndex
					, int k
					, std::vector<double>* intervals) const {
	
	double minValue = dataSet->GetFeature(0, featureIndex);
	double maxValue = dataSet->GetFeature(0, featureIndex);

	for (int objectIndex = 1
		; objectIndex < dataSet->GetObjectCount()
		; objectIndex++) {
		
		minValue = std::min(minValue
			, dataSet->GetFeature(objectIndex, featureIndex));
		
		maxValue = std::max(maxValue
			, dataSet->GetFeature(objectIndex, featureIndex));
	}

	double stepValue = (maxValue - minValue) / k;

	intervals->clear();
	for (int i = 0; i < k; i++, minValue += stepValue) {
		intervals->push_back(minValue);
	}	
	intervals->push_back(DBL_MAX);
}


void EfdDiscretizor::Discretize(
					/* const */ IDataSet* dataSet
					, int featureIndex
					, int k
					, std::vector<double>* intervals) const {

	int minIndex = 0;
	int stepIndex = dataSet->GetObjectCount() / k;

	dataSet->SortObjectsByFeature(featureIndex);

	std::vector<double> current;
	
	while(current.size() < k && stepIndex > 0) {
		
		current.clear();		
		while (minIndex < dataSet->GetObjectCount()) {
			
			current.push_back(dataSet->GetFeature(minIndex, featureIndex));
			minIndex += stepIndex;
			
			if (minIndex >= dataSet->GetObjectCount() - 1) {
				break;
			}
			
			double value = dataSet->GetFeature(minIndex, featureIndex);			
			while (minIndex < dataSet->GetObjectCount() - 1) {
				if (value != dataSet->GetFeature(minIndex + 1, featureIndex)) {
					break;
				}
				++minIndex;
			}
		}

		--stepIndex;
	}

	if (current.empty()) { return; }

	*intervals = current;	
	intervals->push_back(DBL_MAX);
}

void EmdDiscretizor::Discretize(
					/* const */ IDataSet* dataSet
					, int featureIndex
					, int k
					, std::vector<double>* intervals) const {

	dataSet->SortObjectsByFeature(featureIndex);

	std::vector<double> current;
	current.push_back(DBL_MAX);

	while (current.size() < (k + 1)) {

		double maxCutPoint = DBL_MAX;
		double maxInfoGain = 0.;

		double begin = dataSet->GetFeature(0, featureIndex);

		for (int i = 0; i < current.size(); i++) {
			
			double cutPoint = DBL_MAX;
			double infoGain = GetGain(dataSet
				, featureIndex
				, begin
				, current[i]
				, &cutPoint);

			if (maxInfoGain < infoGain) {
				maxInfoGain = infoGain;
				maxCutPoint = cutPoint;
			}

			begin = current[i];
		}

		if (maxCutPoint == DBL_MAX) {
			break; 
		}

		current.resize(current.size() + 1, DBL_MAX);

		{
			int i;
			for (i = current.size() - 1; i > 0; i--) {
				if (current[i - 1] < maxCutPoint) { break; }
				current[i] = current[i - 1];
			}
			current[i] = maxCutPoint;
		}
	}

	if ( current.size() < (k + 1) ) {
		return;
	}

	*intervals = current;
}


double EmdDiscretizor::GetGain(IDataSet* dataSet
							   , int featureIndex
							   , double beginValue
							   , double endValue
							   , double* cutPoint) const {

	struct H {
		double operator() (double p, double n) const {
			double q0 = p / (p + n + 1E-9) + 1E-9;
			double q1 = n / (p + n + 1E-9) + 1E-9;
			return - q0*log(q0) - q1*log(q1);
		}
	};

	H h;

	int objectIndexBegin = 0;

	while (objectIndexBegin < dataSet->GetObjectCount()) {
		if (dataSet->GetFeature(objectIndexBegin, featureIndex) >= beginValue) {
			break;
		} ++objectIndexBegin;
	}

	if (objectIndexBegin == dataSet->GetObjectCount()) { return 0.; }

	int objectIndexEnd = 0;

	while (objectIndexEnd < dataSet->GetObjectCount()) {
		if (dataSet->GetFeature(objectIndexEnd, featureIndex) >= endValue) {
			break;
		} ++objectIndexEnd;
	}

	std::vector<int> objectsCount(dataSet->GetClassCount(), 0);
	
	for (int objectIndex = objectIndexBegin
		; objectIndex < objectIndexEnd
		; objectIndex++) {
		objectsCount[dataSet->GetTarget(objectIndex)] += 1;
	}

	double maxInfoGain = 0.;

	std::vector<int> selectedObjectsCount(dataSet->GetClassCount(), 0);

	selectedObjectsCount[dataSet->GetTarget(objectIndexBegin)] = 1;

	for (int objectIndex = objectIndexBegin + 1
		; objectIndex < objectIndexEnd
		; ++objectIndex) {

		selectedObjectsCount[dataSet->GetTarget(objectIndex)] += 1;

		if (dataSet->GetTarget(objectIndex - 1)
			!= dataSet->GetTarget(objectIndex)			
			&& dataSet->GetFeature(objectIndex - 1, featureIndex)
			< dataSet->GetFeature(objectIndex, featureIndex)) {

			double infoGain = 0.;

			{	// Gain calculation
				int maxTarget = 0;
				double maxObjectsRatio = 0.;
				bool leftFlg = true;
				
				for (int i = 0; i < objectsCount.size(); i++) {
					double objectsRatio = selectedObjectsCount[i] / (objectsCount[i] + 1E-9);
					if (objectsRatio > maxObjectsRatio) {
						maxObjectsRatio = objectsRatio;
						maxTarget = i;
						leftFlg = true;
					}
					if ((1. - objectsRatio) > maxObjectsRatio) {
						maxObjectsRatio = 1. - objectsRatio;
						maxTarget = i;
						leftFlg = false;
					}
				}

				int totalObjects = objectIndex - objectIndexBegin;
				int correctObjects = selectedObjectsCount[maxTarget];
				int incorrectObjects = totalObjects - correctObjects;

				if (!leftFlg) {
					totalObjects = objectIndexEnd - objectIndex - 1;
					correctObjects = objectsCount[maxTarget] - selectedObjectsCount[maxTarget];
					incorrectObjects = totalObjects - correctObjects;
				}

				int targetObjects = objectsCount[maxTarget];
				int nonTargetObjects = objectIndexEnd - objectIndexBegin - targetObjects;

				double a = (correctObjects + incorrectObjects) / (double)(targetObjects + nonTargetObjects);
				double b = (targetObjects + nonTargetObjects - correctObjects - incorrectObjects) / (double)(targetObjects + nonTargetObjects);

				infoGain = 1/ (a * h(correctObjects, incorrectObjects)
					+ b * h(targetObjects - correctObjects, nonTargetObjects - incorrectObjects ));

				infoGain = infoGain;
			}

			if (infoGain > maxInfoGain) {

				maxInfoGain = infoGain;
				
				*cutPoint = dataSet->GetFeature(objectIndex - 1, featureIndex);
				
				*cutPoint += 0.5 * (dataSet->GetFeature(objectIndex, featureIndex)
									- dataSet->GetFeature(objectIndex - 1, featureIndex));
			}
		}
	}

	return maxInfoGain;
}
