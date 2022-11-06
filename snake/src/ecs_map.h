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

namespace ecs
{

template<typename TKey, typename TValue>
struct pair{
    pair(const TKey& key, const TValue& val): first(key), second(val){}
    pair() = default;
    TKey first;
    TValue second;
};

template<typename TKey, typename TValue>
class map
{
public:
    using PairType = pair<TKey, TValue>;
    using iterator = typename list<PairType>::iterator;
    using const_iterator = typename list<PairType>::const_iterator;

    PairType& at(const TKey& key)
    {
        PairType ret;
        typename list<PairType>::iterator it = find_if(storage_.begin(), storage_.end(), 
            [&key](const PairType& kv) { return kv.first == key; });
        assert(it != storage_.end());
        return *it;
    }

    const PairType& at(const TKey& key) const
    {
        return const_cast<map<TKey, TValue>*>(this)->at(key);
    }

    

    size_t size() const { return storage_.size(); }
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

    TValue& operator [] (const TKey& key) 
    { 
        return insert(PairType(key, TValue())).first->second;
    }
private:
    list<PairType> storage_;
};
} // namespace ecs



#endif // _ECS_MAP_H_
