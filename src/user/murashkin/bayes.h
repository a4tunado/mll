#ifndef BAYES_H_
#define BAYES_H_

#include <vector>
#include "core/data.h"
#include "core/logger.h"

namespace mll {
namespace murashkin {

class Bayes {
public:
	enum { EWD, EFD, EMD };
	
	Bayes() : discretizor_(EWD), separate_(true) { }
	
	void Learn(/*const*/ IDataSet* dataSet);

	void Learn(/*const*/ IDataSet* dataSet
					, const std::vector< std::vector<double> >& intervals);

	void GetIntervals(/*const*/ IDataSet* dataSet
					, std::vector< std::vector<double> >* intervals) const;

	int Classify(const FeaturesView& featuresView) const;

	bool GetSeparate() const { return separate_; }
	void SetSeparate(bool separate) { 
		separate_ = separate;
		LOGI("Bayes Separate: %s", (separate_ ? "TRUE" : "FALSE"));
	}

	int GetDiscretizor() const { return discretizor_; }
	void SetDiscretizor(int discretizor) {
		discretizor_ = discretizor;
		LOGI("Bayes Discretizor: %d", discretizor_);
	}
	
	int GetTimes() const { return times_; }
	void SetTimes(int times) {
		times_ = times;
		LOGI("Bayes Times: %d", times_);
	}
	
	int GetQuotient() const { return quotient_; }
	void SetQuotient(int quotient) {
		quotient_ = quotient; 
		LOGI("Bayes Quotient: %d", quotient_);
	}

private:
	void Descretize(IDataSet* dataSet
					, int featureIndex
					, std::vector<double>* intervals) const;

	int GetInterval(const std::vector<double>& intervals, double value) const;

	bool separate_;											//!< separate discretization flag
	int discretizor_;										//!< descretization method
	
	int times_;
	int quotient_;
	
	int object_count_;
	std::vector< std::vector<double> > intervals_;				//!< discretization intervals
	std::vector< std::vector< std::vector<int> > > counts_;		//!< objects per interval count for each class
	std::vector<int> objects_;									//!< object count for each class
};

}
}

#endif // BAYES_H_