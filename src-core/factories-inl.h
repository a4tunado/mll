#ifndef FACTORIES_INL_H_
#error "Direct inclusion of this file is not allowed, include factories.h"
#endif
#undef FACTORIES_INL_H_

namespace mll {

template<typename TFactory, typename TObject>
TFactory& FactoryBase<TFactory, TObject>::Instance() {
    static TFactory factory;
    return factory;
}

template<typename TFactory, typename TObject>
bool FactoryBase<TFactory, TObject>::Register(Entry entry) {
    if (entry.IsEmpty() ||
        entry.GetName().empty() ||
        entry.GetAuthor().empty() ||
        entry.GetDescription().empty())
    {
        return false;
    }
    std::string name = entry.GetName();
    ToLower(&name);
    entries_[name].push_back(entry);
    return true;
}

template<typename TFactory, typename TObject>
bool FactoryBase<TFactory, TObject>::Contains(
    const std::string& name,
    const std::string& author /*= ""*/) const
{
    return !GetEntry(name, author).IsEmpty();
}


template<typename TFactory, typename TObject>
const typename FactoryBase<TFactory, TObject>::Entry& FactoryBase<TFactory, TObject>::GetEntry(
    const std::string& name,
    const std::string& author /*= ""*/) const
{
    std::string localName = name;
    ToLower(&localName);
    typename std::map<std::string, std::vector< Entry > >::const_iterator mapIt = 
        entries_.find(localName);
    if (mapIt == entries_.end()) {
        return Entry::EmptyEntry();
    }
    if (author.length() == 0) {
        return mapIt->second.at(0);
    }
    std::string localAuthor = author;
    ToLower(&localAuthor);
    for (typename std::vector< Entry >::const_iterator vectorIt = mapIt->second.begin();
         vectorIt != mapIt->second.end();
         ++vectorIt) {
        std::string entryAuthor = vectorIt->GetAuthor();
        ToLower(&entryAuthor);
        if (entryAuthor == localAuthor) {
            return *vectorIt;
        }
    }    
    return Entry::EmptyEntry();
}

template<typename TFactory, typename TObject>
sh_ptr<TObject> FactoryBase<TFactory, TObject>::Create(
    const std::string& name,
    const std::string& author /*= ""*/) const
{
    const Entry& entry = GetEntry(name, author);
    return entry.IsEmpty() ? sh_ptr<TObject>(NULL) : entry.GetObject().Clone();
}

template<typename TFactory, typename TObject>
void FactoryBase<TFactory, TObject>::GetEntries(std::vector< Entry >* entries) const {
    for (typename std::map<std::string, std::vector< Entry > >::const_iterator mapIt = entries_.begin();
         mapIt != entries_.end();
         ++mapIt) {
        for (typename std::vector< Entry >::const_iterator vectorIt = mapIt->second.begin();
             vectorIt != mapIt->second.end();
             ++vectorIt) {
            entries->push_back(*vectorIt);
        }
    }
}

} // namespace mll
