#include <gtest/gtest.h>

#include "dataset.h"
#include "dataset_wrapper.h"

using namespace mll;

class DataSetWrapperTest : public testing::Test { };

TEST_F(DataSetWrapperTest, WrappingTest) 
{	
	const int OBJECTS = 1000 + rand() % 500;
	const int FEATURES = 5 + rand() % 10;	
	
	// Creating dataset
	DataSet dataSet;
	dataSet.Resize(OBJECTS, FEATURES);

	ASSERT_TRUE(dataSet.GetObjectCount() == OBJECTS);
	ASSERT_TRUE(dataSet.GetFeatureCount() == FEATURES);
	
	for (int i = 0; i < dataSet.GetObjectCount(); i++) {
		dataSet.SetTarget(i, rand() % 2);
		dataSet.SetWeight(i, (double)rand() / RAND_MAX);
		for (int j = 0; j < dataSet.GetFeatureCount(); j++) {
			dataSet.SetFeature(i, j, (double)rand() / RAND_MAX);
		}
	}

	std::vector<int> indexes(OBJECTS / 2);
	for (int i = 0; i < (int)indexes.size(); i++) {
		indexes[i] = i;
	}

	// Creating first wrapper
	DataSetWrapper testSet(&dataSet);
	testSet.SetObjectIndexes(indexes.begin(), indexes.end());
	ASSERT_TRUE(testSet.GetObjectCount() == indexes.size());

	// Wrapper first wrapper
	// NOTE: casting is needed here to avoid calling copy constructor
	DataSetWrapper testSetWrapper(&testSet);
	ASSERT_TRUE(testSetWrapper.GetObjectCount() == indexes.size());

	testSetWrapper.ResetObjectIndexes();
	ASSERT_TRUE(testSetWrapper.GetObjectCount() == indexes.size());
	
	testSetWrapper.SortObjectsByWeight();
	ASSERT_TRUE(testSetWrapper.GetObjectCount() == indexes.size());

	testSetWrapper.ResetObjectIndexes();
	ASSERT_TRUE(testSetWrapper.GetObjectCount() == indexes.size());
}
