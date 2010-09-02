#ifndef DISCRETIZOR_H_
#define DISCRETIZOR_H_

#include <vector>
#include "core/data.h"
#include "core/dataset_wrapper.h"

namespace mll {

class Discretizor {
public:
	typedef std::vector< std::vector<double> > Intervals;

	struct IQualifier {
		virtual ~IQualifier() { }
		virtual double operator()(
			/*const*/IDataSet* train
			, /*const*/IDataSet* test
			, const Intervals& intervals) const = 0;
	};

	virtual ~Discretizor() { }

	void Discretize(/*const*/ IDataSet* dataSet
					, int times
					, int quotient
					, const IQualifier& qualifier
					, Intervals* intervals) const;
protected:

	virtual void Discretize(
					/* const */ IDataSet* dataSet
					, int featureIndex
					, int k
					, std::vector<double>* intervals) const = 0;
};

class EwdDiscretizor : public Discretizor {
protected:

	virtual void Discretize(
					/* const */ IDataSet* dataSet
					, int featureIndex
					, int k
					, std::vector<double>* intervals) const;
};

class EfdDiscretizor : public Discretizor {
protected:

	virtual void Discretize(
					/* const */ IDataSet* dataSet
					, int featureIndex
					, int k
					, std::vector<double>* intervals) const;
};

class EmdDiscretizor : public Discretizor {
protected:

	virtual void Discretize(
					/* const */ IDataSet* dataSet
					, int featureIndex
					, int k
					, std::vector<double>* intervals) const;

	double GetGain(IDataSet* trainDataSet
							   , int featureIndex
							   , double begin
							   , double end
							   , double* cut) const;
};

}

#endif // DISCRETIZOR_H_