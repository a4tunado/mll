#include <gtest/gtest.h>

#include "dataset.h"
#include "dataset_wrapper.h"

using namespace mll;

class DataSetWrapperTest : public testing::Test { };

//! Wrapping test
//  Shows that wrapper constructor argument should be a pointer rather than reference
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

//! Lazy coping test
//  1. Creates data set and initializes targets, weights and features within a given range.
//  2. Creates wrapper and changes values to the shifted range
//  3. Check that changes didn't affect to the origginal data set 
//  4. Reset wrapper and check that original values are still the same
TEST_F(DataSetWrapperTest, LazyCopingTest) 
{	
	#define RAND(min, max) min + (max - min) * rand() / (double)RAND_MAX
	#define ASSERT_INRANGE(x, min, max) ASSERT_TRUE(x >= min && x <= max)

	const int OBJECTS = 1000 + rand() % 500;
	const int FEATURES = 5 + rand() % 10;

	const int ORIGINAL_TARGET_MIN = 0;
	const int ORIGINAL_TARGET_MAX = ORIGINAL_TARGET_MIN + 10 + rand() % 10;

	const int WRAPPER_TARGET_MIN = 100;
	const int WRAPPER_TARGET_MAX = WRAPPER_TARGET_MIN + 10 + rand() % 10;

	const int ORIGINAL_WEIGHT_MIN = 0;
	const int ORIGINAL_WEIGHT_MAX = ORIGINAL_WEIGHT_MIN + 10 + rand() % 10;

	const int WRAPPER_WEIGHT_MIN = 100;
	const int WRAPPER_WEIGHT_MAX = WRAPPER_WEIGHT_MIN + 10 + rand() % 10;
	
	const int ORIGINAL_FEATURE_MIN = 0;
	const int ORIGINAL_FEATURE_MAX = ORIGINAL_FEATURE_MIN + 50 + rand() % 10;
	
	const int WRAPPER_FEATURE_MIN = 100;
	const int WRAPPER_FEATURE_MAX = WRAPPER_FEATURE_MIN + 50 + rand() % 10;
	
	// Creating dataset
	DataSet dataSet;
	dataSet.Resize(OBJECTS, FEATURES);

	ASSERT_TRUE(dataSet.GetObjectCount() == OBJECTS);
	ASSERT_TRUE(dataSet.GetFeatureCount() == FEATURES);

	dataSet.GetMetaData().SetTargetInfo(
		FeatureInfo("Test"
			, FeatureType::Nominal
			, false
			, std::vector<std::string>(std::max(ORIGINAL_TARGET_MAX, WRAPPER_TARGET_MAX))));
	
	for (int i = 0; i < dataSet.GetObjectCount(); i++) {
		dataSet.SetTarget(i, (int)(RAND(ORIGINAL_TARGET_MIN, ORIGINAL_TARGET_MAX)));
		dataSet.SetWeight(i, RAND(ORIGINAL_WEIGHT_MIN, ORIGINAL_WEIGHT_MAX));
		for (int j = 0; j < dataSet.GetFeatureCount(); j++) {
			dataSet.SetFeature(i, j, RAND(ORIGINAL_FEATURE_MIN, ORIGINAL_FEATURE_MAX));
		}
	}

	for (int i = 0; i < dataSet.GetObjectCount(); i++) {
		ASSERT_INRANGE(dataSet.GetTarget(i), ORIGINAL_TARGET_MIN, ORIGINAL_TARGET_MAX);
		ASSERT_INRANGE(dataSet.GetWeight(i), ORIGINAL_WEIGHT_MIN, ORIGINAL_WEIGHT_MAX);
		for (int j = 0; j < dataSet.GetFeatureCount(); j++) {
			ASSERT_INRANGE(dataSet.GetFeature(i, j), ORIGINAL_FEATURE_MIN, ORIGINAL_FEATURE_MAX);
		}
	}

	// Creating wrapper
	DataSetWrapper wrapper(&dataSet);
	ASSERT_TRUE(wrapper.GetObjectCount() == dataSet.GetObjectCount());
	ASSERT_TRUE(wrapper.GetFeatureCount() == dataSet.GetFeatureCount());

	for (int i = 0; i < wrapper.GetObjectCount(); i++) {
		wrapper.SetTarget(i, (int)(RAND(WRAPPER_TARGET_MIN, WRAPPER_TARGET_MAX)));
		wrapper.SetWeight(i, RAND(WRAPPER_WEIGHT_MIN, WRAPPER_WEIGHT_MAX));
		for (int j = 0; j < wrapper.GetFeatureCount(); j++) {
			wrapper.SetFeature(i, j, RAND(WRAPPER_FEATURE_MIN, WRAPPER_FEATURE_MAX));
		}
	}

	for (int i = 0; i < wrapper.GetObjectCount(); i++) {
		ASSERT_INRANGE(wrapper.GetTarget(i), WRAPPER_TARGET_MIN, WRAPPER_TARGET_MAX);
		ASSERT_INRANGE(wrapper.GetWeight(i), WRAPPER_WEIGHT_MIN, WRAPPER_WEIGHT_MAX);
		for (int j = 0; j < wrapper.GetFeatureCount(); j++) {
			ASSERT_INRANGE(wrapper.GetFeature(i, j), WRAPPER_FEATURE_MIN, WRAPPER_FEATURE_MAX);
		}
	}

	for (int i = 0; i < dataSet.GetObjectCount(); i++) {
		ASSERT_INRANGE(dataSet.GetTarget(i), ORIGINAL_TARGET_MIN, ORIGINAL_TARGET_MAX);
		ASSERT_INRANGE(dataSet.GetWeight(i), ORIGINAL_WEIGHT_MIN, ORIGINAL_WEIGHT_MAX);
		for (int j = 0; j < dataSet.GetFeatureCount(); j++) {
			ASSERT_INRANGE(dataSet.GetFeature(i, j), ORIGINAL_FEATURE_MIN, ORIGINAL_FEATURE_MAX);
		}
	}

	wrapper.Reset();

	for (int i = 0; i < wrapper.GetObjectCount(); i++) {
		ASSERT_INRANGE(wrapper.GetTarget(i), ORIGINAL_TARGET_MIN, ORIGINAL_TARGET_MAX);
		ASSERT_INRANGE(wrapper.GetWeight(i), ORIGINAL_WEIGHT_MIN, ORIGINAL_WEIGHT_MAX);
		for (int j = 0; j < wrapper.GetFeatureCount(); j++) {
			ASSERT_INRANGE(wrapper.GetFeature(i, j), ORIGINAL_FEATURE_MIN, ORIGINAL_FEATURE_MAX);
		}
	}

	#undef RAND
	#undef ASSERT_INRANGE
}

//! Sorting test
TEST_F(DataSetWrapperTest, SortingTest) 
{	
	const int OBJECTS = 100 + rand() % 50;
	const int FEATURES = 2 + rand() % 10;	
	const int TARGETS = 3 + rand() % 10;
	
	// Creating dataset
	DataSet dataSet;
	dataSet.Resize(OBJECTS, FEATURES);

	dataSet.GetMetaData().SetTargetInfo(
		FeatureInfo("Test"
			, FeatureType::Nominal
			, false
			, std::vector<std::string>(TARGETS)));

	ASSERT_TRUE(dataSet.GetObjectCount() == OBJECTS);
	ASSERT_TRUE(dataSet.GetFeatureCount() == FEATURES);
	
	for (int i = 0; i < dataSet.GetObjectCount(); i++) {
		dataSet.SetTarget(i, rand() % TARGETS);
		dataSet.SetWeight(i, (double)rand() / RAND_MAX);
		for (int j = 0; j < dataSet.GetFeatureCount(); j++) {
			dataSet.SetFeature(i, j, (double)rand() / RAND_MAX);
		}
	}

	// Creating wrapper
	DataSetWrapper wrapper(&dataSet);
	ASSERT_TRUE(dataSet.GetObjectCount() == wrapper.GetObjectCount());
	ASSERT_TRUE(dataSet.GetFeatureCount() == wrapper.GetFeatureCount());

	// Sorting by target
	wrapper.SortObjectsByTarget();
	for (int i = 1; i < wrapper.GetObjectCount(); i++) {
		ASSERT_TRUE(wrapper.GetTarget(i-1) <= wrapper.GetTarget(i));
	}
	wrapper.SortObjectsByTarget(true);
	for (int i = 1; i < wrapper.GetObjectCount(); i++) {
		ASSERT_TRUE(wrapper.GetTarget(i-1) >= wrapper.GetTarget(i));
	}

	// Sorting by weight
	wrapper.SortObjectsByWeight();
	for (int i = 1; i < wrapper.GetObjectCount(); i++) {
		ASSERT_TRUE(wrapper.GetWeight(i-1) <= wrapper.GetWeight(i));
	}
	wrapper.SortObjectsByWeight(true);
	for (int i = 1; i < wrapper.GetObjectCount(); i++) {
		ASSERT_TRUE(wrapper.GetWeight(i-1) >= wrapper.GetWeight(i));
	}

	// Sorting by features
	for (int j = 0; j < wrapper.GetFeatureCount(); j++) {
		wrapper.SortObjectsByFeature(j);
		for (int i = 1; i < wrapper.GetObjectCount(); i++) {
			ASSERT_TRUE(wrapper.GetFeature(i-1, j) <= wrapper.GetFeature(i, j));
		}
		wrapper.SortObjectsByFeature(j, true);
		for (int i = 1; i < wrapper.GetObjectCount(); i++) {
			ASSERT_TRUE(wrapper.GetFeature(i-1, j) >= wrapper.GetFeature(i, j));
		}
	}
}