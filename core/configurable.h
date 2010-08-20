#ifndef CONFIGURABLE_H_
#define CONFIGURABLE_H_

#include <iostream>

#include "sh_ptr.h"
#include "util.h"

namespace mll {

#pragma warning(disable: 4250) // Wrong MSVC warning about virtual diamond inheritance

class IConfigurable {
public:
    virtual void PrintParameters(bool printValues, bool printDescriptions) const = 0;
    virtual std::string GetParameter(const std::string& parameterName) const = 0;
    virtual bool SetParameter(const std::string& parameterName, const std::string& parameterValue) = 0;
    virtual ~IConfigurable() {
    }
};

template<typename TObjectType>
class IParameter {
public:
    virtual const std::string& GetName() const = 0;
    virtual const std::string& GetDescription() const = 0;
    virtual std::string GetValue(const TObjectType* object) const = 0;
    virtual void SetValue(TObjectType* object, std::string value) const = 0;
    virtual ~IParameter() {
    }
};

template<typename TObjectType>
class Configurable: public virtual IConfigurable {
public:
    virtual void PrintParameters(bool printValues, bool printDescriptions) const;
    virtual std::string GetParameter(const std::string& parameterName) const;
    virtual bool SetParameter(const std::string& parameterName, const std::string& parameterValue);
    
protected:
    template<typename TParameterType,
             typename TGetter,
             typename TSetter> 
    void AddParameter(const std::string& name,
                      TParameterType initialValue,
                      TGetter getter,
                      TSetter setter,
                      const std::string& description);

private:
    typedef sh_ptr< IParameter<TObjectType> > ParameterPtr;

    std::vector<ParameterPtr> parameters_;
};

template<typename TObjectType,
         typename TParameterType,
         typename TGetter,
         typename TSetter>
class Parameter: public IParameter<TObjectType> {
private:
    std::string name_;
    TGetter getter_;
    TSetter setter_;
    std::string description_;

public:
    Parameter(const std::string& name,
              TGetter getter,
              TSetter setter,
              const std::string& description)
        : name_(name),
          getter_(getter),
          setter_(setter),
          description_(description) {
    }

    virtual const std::string& GetName() const {
        return name_;
    }

    virtual std::string GetValue(const TObjectType* object) const {
        return ToString((object->*getter_)());
    }

    virtual void SetValue(TObjectType* object, std::string value) const {
        (object->*setter_)(FromString<TParameterType>(value));
    }

    virtual const std::string& GetDescription() const {
        return description_;
    }
};

} // namespace mll

#define CONFIGURABLE_INL_H_
#include "configurable-inl.h"
#undef CONFIGURABLE_INL_H_

#endif // CONFIGURABLE_H_
