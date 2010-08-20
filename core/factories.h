#ifndef FACTORIES_H_
#define FACTORIES_H_

#include <map>
#include <set>
#include <string>

#include "sh_ptr.h"
#include "classifier.h"
#include "tester.h"

//! Factory registration flag
#define DECLARE_REGISTRATION() static const bool REGISTERED

//! Macro for classifier registration in the factory
#define REGISTER_CLASSIFIER(type, name, author, description)    \
    const bool type::REGISTERED =                               \
        RegisterClassifier<type>(name, author, description)

//! Macro for tester registration in the factory
#define REGISTER_TESTER(type, name, author, description)        \
    const bool type::REGISTERED =                               \
        RegisterTester<type>(name, author, description)    

namespace mll {

//! Factory entry
template<typename TObject>
class FactoryEntry {
public:
    static const FactoryEntry& EmptyEntry() {
        static FactoryEntry emptyEntry(sh_ptr<TObject>(), "", "", "");
        return emptyEntry;
    }

    FactoryEntry(
        sh_ptr<TObject> object,
        const std::string& name,
        const std::string& author,
        const std::string& description)
        : object_(object),
          name_(name),
          author_(author),
          description_(description) {
    }

    const TObject& GetObject() const {
        return *object_;
    }

    const std::string& GetName() const {
        return name_;
    }

    const std::string& GetAuthor() const {
        return author_;
    }

    const std::string& GetDescription() const {
        return description_;
    }

    bool IsEmpty() const {
        return object_.get() == NULL;
    }
    
private:
    sh_ptr<TObject> object_;
    std::string name_;
    std::string author_;
    std::string description_;
};

//! Factory base class
template<typename TFactory, typename TObject>
class FactoryBase {
public:	
    typedef FactoryEntry<TObject> Entry;

	static TFactory& Instance();

    bool Register(Entry entry);
    bool Contains(const std::string& name,
                  const std::string& author = "") const;
    const Entry& GetEntry(const std::string& name,
                          const std::string& author = "") const;
	sh_ptr<TObject> Create(const std::string& name,
                           const std::string& author = "") const;
	void GetEntries(std::vector< Entry >* entries) const;

    virtual ~FactoryBase() {
    }

protected:
    FactoryBase() {
    }

private:
    std::map<std::string, std::vector< Entry > > entries_;
};

//! IClassifier factory
class ClassifierFactory: public FactoryBase<ClassifierFactory, IClassifier> {
};

//! ITester factory
class TesterFactory: public FactoryBase<TesterFactory, ITester> {
};

template<typename TClassifier>
inline bool RegisterClassifier(
    const std::string& name,
    const std::string& author,
    const std::string& description) 
{
    return ClassifierFactory::Instance().Register(ClassifierFactory::Entry(
        sh_ptr<IClassifier>(new TClassifier()), name, author, description));
}

template<typename TTester>
inline bool RegisterTester(
    const std::string& name,
    const std::string& author,
    const std::string& description) 
{
    return TesterFactory::Instance().Register(TesterFactory::Entry(
        sh_ptr<ITester>(new TTester()), name, author, description));
}

} // namespace mll

#define FACTORIES_INL_H_
#include "factories-inl.h"
#undef FACTORIES_INL_H_

#endif  // FACTORY_H_
