#include "metadata_wrapper.h"

#include <stdexcept>

using std::vector;

namespace mll {

void MetaDataWrapper::SetFeatureIndexes(sh_ptr< vector<int> > indexes) {
    for (vector<int>::const_iterator it = indexes->begin(); it != indexes->end(); ++it) {
        if (*it < 0 || *it >= metaData_->GetFeatureCount()) {
            throw std::out_of_range("Indexes was out of range");
        }
    }
    featureIndexes_ = indexes;
}

} // namespace mll
