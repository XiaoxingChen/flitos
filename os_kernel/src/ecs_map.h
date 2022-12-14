/**
******************************************************************************
* @file    ecs_map.h
* @author  Chenxx
* @version V1.0
* @date    2022-11-05
* @brief   simple ordered map implementation, O(n) insertion and query.
******************************************************************************/
#if !defined(_ECS_MAP_H_)
#define _ECS_MAP_H_

#include "ecs_list.h"
#include "ecs_assert.h"
#include "ecs_utility.h"

namespace ecs
{

template<typename TKey, typename TValue>
class map
{
public:
    using PairType = pair<TKey, TValue>;
    using iterator = typename list<PairType>::iterator;
    using const_iterator = typename list<PairType>::const_iterator;

    TValue& at(const TKey& key)
    {
        TValue ret;
        typename list<PairType>::iterator it = find_if(storage_.begin(), storage_.end(), 
            [&key](const PairType& kv) { return kv.first == key; });
        assert(it != storage_.end());
        return it->second;
    }

    const TValue& at(const TKey& key) const
    {
        return const_cast<map<TKey, TValue>*>(this)->at(key);
    }

    

    size_t size() const { return storage_.size(); }
    void clear() { storage_.clear(); }

    void erase(const TKey& key)
    {
        typename list<PairType>::iterator it = find_if(storage_.begin(), storage_.end(), 
            [&key](const PairType& kv) { return kv.first == key; });
        if(it != storage_.end())
        {
            storage_.erase(it);
        }
    }

    pair<iterator, bool> insert(const PairType& pair_in)
    {
        pair<iterator, bool> ret;
        typename list<PairType>::iterator it = find_if(storage_.begin(), storage_.end(), 
            [&pair_in](const PairType& kv) { return kv.first >= pair_in.first; });
        if(it == storage_.end())
        {
            storage_.push_back(pair_in);
            ret.first = storage_.end();
            ret.first--;
            ret.second = true;
            return ret;
        }
        if(it->first == pair_in.first)
        {
            ret.first = it;
            ret.second = false;
            return ret;
        }

        ret.first = storage_.insert(it, pair_in);
        ret.second = true;
        return ret;
    }

    iterator insert(iterator pos, const PairType& pair_in)
    {
        return storage_.insert(pos, pair_in);
    }

    TValue& operator [] (const TKey& key) 
    { 
        pair<iterator, bool> ret;
        typename list<PairType>::iterator it = find_if(storage_.begin(), storage_.end(), 
            [&key](const PairType& kv) { return kv.first >= key; });
        if(it != storage_.end() && it->first == key)
        {
            return it->second;
        }

        return insert(it, PairType(key, TValue()))->second;
    }
#if 1
    size_t count(const TKey& key) const 
    {
        typename list<PairType>::const_iterator it = find_if(storage_.begin(), storage_.end(), 
            [&key](const PairType& kv) { return kv.first >= key; });
        if(it == storage_.end() || it->first > key) return 0;
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
    list<PairType> storage_;
};
} // namespace ecs



#endif // _ECS_MAP_H_
