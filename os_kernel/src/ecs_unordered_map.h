#if !defined(_ECS_UNORDERED_MAP_H_)
#define _ECS_UNORDERED_MAP_H_

#include "ecs_vector.h"
#include "ecs_utility.h"
#include "ecs_assert.h"

namespace ecs
{

template<typename TKey, typename TValue, class Allocator>
class unordered_map
{
public:
    using PairType = pair<TKey, TValue>;
    using iterator = PairType*;
    using const_iterator = const PairType*;
    using ThisType = unordered_map<TKey, TValue, Allocator>;

    TValue& at(const TKey& key)
    {
        TValue ret;
        auto it = find(key);
        assert(it != storage_.end());
        return it->second;
    }

    const TValue& at(const TKey& key) const
    {
        return const_cast<ThisType*>(this)->at(key);
    }

    size_t size() const { return storage_.size(); }
    void clear() { storage_.clear(); }

    void erase(const TKey& key)
    {
        auto it = find(key);
        if(it != storage_.end())
        {
            it->~PairType();
            *it = storage_.at(storage_.size() - 1);
            storage_.pop_back();
        }
    }

    pair<iterator, bool> insert(const PairType& pair_in)
    {
        pair<iterator, bool> ret;
        auto it = find(pair_in.first);
        if(it == storage_.end())
        {
            storage_.push_back(pair_in);
            ret.first = &storage_.at(storage_.size() - 1);
            ret.second = true;
            return ret;
        }
        ret.first = it;
        ret.second = false;
        return ret;
    }

    TValue& operator [] (const TKey& key) 
    { 
        auto it = find(key);
        if(it != storage_.end())
        {
            return it->second;
        }

        storage_.push_back(PairType(key, TValue()));
        return storage_.back().second;
    }
#if 1
    size_t count(const TKey& key) const 
    {
        auto it = find(key);
        if(it == storage_.end()) return 0;
        return 1; /* it.first == key */
    }
#endif

    iterator find(const TKey& key)
    {
        return find_if(storage_.begin(), storage_.end(), 
            [&key](const PairType& kv) { return kv.first == key; });
    }

    const_iterator find(const TKey& key) const
    {
        return find_if(storage_.begin(), storage_.end(), 
            [&key](const PairType& kv) { return kv.first == key; });
    }

    iterator begin() { return storage_.begin(); }
    const_iterator begin() const { return storage_.begin(); }

    iterator end() { return storage_.end(); }
    const_iterator end() const { return storage_.end(); }

private:
    vector<PairType, Allocator> storage_;

};



} // namespace ecs


#endif // _ECS_UNORDERED_MAP_H_
