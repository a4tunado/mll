#include "bayes.h"

#include <limits>
#include <cassert>
#include <algorithm>
#include <fstream>

#include "discretizor.h"
#include "core/dataset_wrapper.h"
#include "core/logger.h"

using namespace mll;
using namespace murashkin;

struct Qualifier : Discretizor::IQualifier {
	
	double operator()(
			/*const*/IDataSet* train
			, /*const*/IDataSet* test
			, const Discretizor::Intervals& intervals) const;

};

double Qualifier::operator()(
			/*const*/IDataSet* trainDataSet
			, /*const*/IDataSet* testDataSet
			, const Discretizor::Intervals& intervals) const {

	std::ofstream logger("qualifier.txt", std::ios_base::app);
	std::ofstream logger_data("qualifier.dat", std::ios_base::app);

	Bayes bayes;
	bayes.Learn(trainDataSet, intervals);

	std::vector<int> predicted(testDataSet->GetObjectCount());

	double error = 0.;
	for (int i = 0; i < testDataSet->GetObjectCount(); i++) {
		int expected = testDataSet->GetTarget(i);
		int actual = bayes.Classify(FeaturesView(testDataSet, i));
		predicted[i] = actual;
		if (actual != expected) {
			error += 1.;
		}
	}
	error /= testDataSet->GetObjectCount();

	logger << "================================" << std::endl;
	
	logger << "Error: " << error << std::endl;

	{
		int k = 0;
		for (int i = 0; i < intervals.size(); i++) {
			k += intervals[i].size();
		}
		logger_data << k << "\t";
	}

	logger_data << error;

	
	
	for (int i = 0; i < intervals.size(); i++) {
		logger << "[" << intervals[i].size() << "]" << "\t";
		for (int j = 0; j < intervals[i].size(); j++) 
		{ logger << intervals[i][j] << "\t"; }
		logger << std::endl;
	}
	
	std::vector<int> confusion(
		testDataSet->GetClassCount()*testDataSet->GetClassCount(), 0);

	for (int i = 0; i < testDataSet->GetObjectCount(); i++) {
		int row = testDataSet->GetTarget(i);
		int col = predicted[i];
		confusion[row*testDataSet->GetClassCount() + col] += 1;
	}
	logger << "Confusion matrix: " << std::endl;
	for (int i = 0; i < testDataSet->GetClassCount(); i++) {
		for (int j = 0; j < testDataSet->GetClassCount(); j++) {
			logger << confusion[i*testDataSet->GetClassCount() + j] << "\t";
		}
		logger << std::endl;
	}

	if (testDataSet->GetClassCount() == 2) {	
		
		int trueNegative = confusion[1*testDataSet->GetClassCount() + 0];
		int falsePositive = confusion[0*testDataSet->GetClassCount() + 1];		
		int truePositive = confusion[0*testDataSet->GetClassCount() + 0];
		int falseNegative = confusion[1*testDataSet->GetClassCount() + 1];

		double recall = truePositive / (truePositive + trueNegative + 1e-9);
		double precision = truePositive / (truePositive + falsePositive + 1e-9);
		double f1 = precision * recall / (precision + recall + 1e-9);

		double error1 = trueNegative / (trueNegative + truePositive + 1e-9);
		double error2 = falsePositive / (falsePositive + falseNegative + 1e-9);

		logger_data << "\t" << f1 << "\t" << error1 << "\t" << error2;
		logger << "F1: " << f1 << "; Err1: " << error1 << "; Err2: " << error2;
	}

	logger_data << std::endl;

	logger << std::endl;

	return error;
}

void Bayes::Learn(/*const*/ IDataSet* dataSet) {

	std::auto_ptr<Discretizor> discretizor;

	switch (GetDiscretizor()) {
		case EMD: { discretizor.reset(new EmdDiscretizor()); break; }
		case EFD: { discretizor.reset(new EfdDiscretizor()); break; }
		default:  { discretizor.reset(new EwdDiscretizor()); break; } 
	}

	intervals_.clear();

	if (GetSeparate()) {
		DataSetWrapper wrapper(dataSet);
		std::vector< std::vector<double> > intervals;
		for (int i = 0; i < dataSet->GetFeatureCount(); i++) {
			wrapper.SetFeatureIndexes(&i, &i + 1);
			discretizor->Discretize(&wrapper
								, GetTimes()
								, GetQuotient()
								, Qualifier()
								, &intervals);
			intervals_.push_back(intervals.front());
		}		
	}
	else {
		discretizor->Discretize(dataSet
								, GetTimes()
								, GetQuotient()
								, Qualifier()
								, &intervals_);
	}
	
	Learn(dataSet, intervals_);

	{
		FILE* file = fopen("intervals.txt", "w");
		for (int i = 0; i < intervals_.size(); i++) {
			fprintf(file, "[%02d] [%02d] ", i, (intervals_[i].size() - 1));
			for (int j = 0; j < intervals_[i].size() - 1; j++) {
				fprintf(file, "\t%.3f", intervals_[i][j]);
			}
			fprintf(file, "\n");
		}
		fclose(file);
	}
}

void Bayes::Learn(/*const*/ IDataSet* dataSet
				  , const std::vector< std::vector<double> >& intervals) {

	intervals_ = intervals;

	objects_.clear();
	objects_.resize(dataSet->GetClassCount(), 0);
	
	counts_.clear();
	counts_.resize(dataSet->GetClassCount());

	for (int targetIndex = 0
		; targetIndex < dataSet->GetClassCount()
		; targetIndex++) {			
		counts_[targetIndex].resize(dataSet->GetFeatureCount());
		for (int featureIndex = 0
			; featureIndex < dataSet->GetFeatureCount()
			; featureIndex++) {
			counts_[targetIndex][featureIndex].resize(intervals_[featureIndex].size(), 0);
		}
	}

	for (int objectIndex = 0
		; objectIndex < dataSet->GetObjectCount()
		; objectIndex++) {
		int targetIndex = dataSet->GetTarget(objectIndex);
		std::vector< std::vector<int> >& counts = counts_[targetIndex];
		for (int featureIndex = 0
			; featureIndex < dataSet->GetFeatureCount()
			; featureIndex++) {
			const double& value = dataSet->GetFeature(objectIndex, featureIndex);
			int interval = GetInterval(intervals_[featureIndex], value);
			counts[featureIndex][interval] += 1;
		}
	}

	for (int targetIndex = 0
		; targetIndex < counts_.size()
		; targetIndex++) {
		int count = 0;
		const std::vector<int>& counts = counts_[targetIndex].front();
		for (int interval = 0
			; interval < counts.size()
			; interval++) {
			count += counts[interval];
		}
		objects_[targetIndex] = count;		
	}
	object_count_ = dataSet->GetObjectCount();
}

int Bayes::Classify(const FeaturesView& featuresView) const {

	std::vector<double> probabilities;
	for (int targetIndex = 0
		; targetIndex < counts_.size()
		; targetIndex++) {
		double probability = 1.;
		//double probability = 
		//	(objects_[targetIndex] + 1) / (object_count_ + counts_.size());		
		for (int featureIndex = 0
			; featureIndex < counts_[targetIndex].size()
			; featureIndex++) {
			const double& value = featuresView.GetFeature(featureIndex);
			int interval = GetInterval(intervals_[featureIndex], value);
			const std::vector<int>& counts = counts_[targetIndex][featureIndex];
			double count = counts[interval];	// objects in interval with given target
			// regularization is used for probability calculation
			probability *= (count + 1.) / (objects_[targetIndex] + intervals_[featureIndex].size());
			// probability *= count / objects_[targetIndex];
		}
		probabilities.push_back(probability);
	}
	typedef std::vector<double>::const_iterator TCIter;
	TCIter it = std::max_element(probabilities.begin(), probabilities.end());
	assert(it != probabilities.end());
	return it - probabilities.begin();
}

int Bayes::GetInterval(const std::vector<double>& intervals, double value) const {
	typedef std::vector<double>::const_iterator TCIter;			
	TCIter it = std::upper_bound(intervals.begin(), intervals.end(), value);
	assert(it != intervals.end());
	return (it - intervals.begin());
}
