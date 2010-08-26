#ifndef CONFIGURABLE_INL_H_
#error "Direct inclusion of this file is not allowed, include configurable.h"
#endif
#undef CONFIGURABLE_INL_H_

namespace mll {

template<typename TObjectType>
void Configurable<TObjectType>::PrintParameters(bool printValues, bool printDescriptions) const {
    for (typename std::vector<ParameterPtr>::const_iterator it = parameters_.begin();
         it != parameters_.end(); 
         ++it)
    {
        std::cout << (*it)->GetName();
        if (printDescriptions) {
            std::cout << " (" << (*it)->GetDescription() << ")";
        }
        if (printValues) {
            std::cout << ": " << (*it)->GetValue(dynamic_cast<const TObjectType*>(this));
        }
        std::cout << std::endl;
    }
}

template<typename TObjectType>
std::string Configurable<TObjectType>::GetParameter(const std::string& parameterName) const {
    for (typename std::vector<ParameterPtr>::const_iterator it = parameters_.begin();
         it != parameters_.end();
         ++it)
    {
        if ((*it)->GetName() == parameterName) {
            return (*it)->GetValue(dynamic_cast<const TObjectType*>(this));
        }
    }
    return std::string();
}

template<typename TObjectType>
bool Configurable<TObjectType>::SetParameter(const std::string& parameterName, const std::string& parameterValue) {
    for (typename std::vector<ParameterPtr>::const_iterator it = parameters_.begin();
         it != parameters_.end();
         ++it)
    {
        if ((*it)->GetName() == parameterName) {
            (*it)->SetValue(dynamic_cast<TObjectType*>(this), parameterValue);
            return true;
        }
    }
    return false; 
}

template<typename TObjectType>
template<typename TParameterType,
         typename TGetter,
         typename TSetter> 
void Configurable<TObjectType>::AddParameter(const std::string& name,
                                             TParameterType /*initialValue*/,
                                             TGetter getter,
                                             TSetter setter,
                                             const std::string& description) {
    parameters_.push_back(ParameterPtr(
        new Parameter<TObjectType, TParameterType, TGetter, TSetter>(
            name, getter, setter, description)));
}


} // namespace mll
