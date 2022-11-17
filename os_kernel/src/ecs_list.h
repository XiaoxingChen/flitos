/**
******************************************************************************
* @file    ecs_list.h
* @author  Chenxx
* @version V1.0
* @date    2022-11-05
* @brief   simple doubly linked list implementation.
******************************************************************************/
#if !defined(_ECS_LIST_H_)
#define _ECS_LIST_H_

namespace ecs
{

template<typename T>
class list
{
private:
    template<typename U>
    struct Node
    {
        Node(const U& val_in): val(val_in) {}
        Node() = default;
        U val;
        Node* prev = nullptr;
        Node* next = nullptr;
    };

    template<typename TNode>
    class Iterator_
    {
    template<typename TValue> friend class list;
    public:
        Iterator_<TNode>& operator++(int) 
        { 
            do{
                if(!p_) break;
                if(reach_end_) break;
                if(p_->next)
                {
                    p_ = p_->next;
                }else
                {
                    reach_end_ = true;
                }
            }while(0);
            return *this;
        }
        Iterator_<TNode>& operator--(int) 
        { 
            do
            {
                if(!p_) break;
                if(reach_end_)
                {
                    reach_end_ = false;
                    break;
                }
                if(p_->prev)
                {
                    p_ = p_->next;
                }
            } while (0);
            return *this;
        }
        bool operator == ( const Iterator_<TNode>& rhs ) const 
        { 
            if(p_ == nullptr && rhs.p_ == nullptr) return true;
            return p_ == rhs.p_ && reach_end_ == rhs.reach_end_; 
        }
        bool operator != ( const Iterator_<TNode>& rhs ) const { return ! (*this == rhs); }
        
        // https://stackoverflow.com/questions/57718018/how-do-i-convert-iterator-to-const-iterator-in-my-custom-list-iterator-class/57718423#57718423
        auto & operator * () { return p_->val; }
        auto * operator -> () { return &p_->val; }

        Iterator_() = default;
        Iterator_(const Iterator_<TNode>& rhs): p_(rhs.p_), reach_end_(rhs.reach_end_) { }
        Iterator_(TNode* rhs): p_(rhs), reach_end_(false) {}

        void operator = (const Iterator_<TNode>& rhs) { p_ = rhs.p_; }
    private:
        TNode* p_ = nullptr;
        bool reach_end_ = true;
    };

    public:
    using const_iterator = Iterator_<const Node<T>>;
    using iterator = Iterator_<Node<T>>;

public:
    void push_back(const T& val)
    {
        if(size_ == 0)
        {
            begin_ = new Node<T>(val);
            rbegin_ = begin_;
        }else
        {
            rbegin_->next = new Node<T>(val);
            rbegin_->next->prev = rbegin_;
            rbegin_ = rbegin_->next;
        }
        size_++;
    }

    void push_front(const T& val)
    {
        if(size_ == 0)
        {
            begin_ = new Node<T>(val);
            rbegin_ = begin_;
        }else
        {
            begin_->prev = new Node<T>(val);
            begin_->prev->next = begin_;
            begin_ = begin_->prev;
        }
        size_++;
    }

    void pop_back()
    {
        if(size_ == 0) return;
        if(size_ == 1)
        {
            free(begin_);
            begin_ = nullptr;
            rbegin_ = nullptr;
            size_ --;
            return ;
        }
        Node<T>* new_rbegin = rbegin_->prev;
        free(new_rbegin->next);
        new_rbegin->next = nullptr;
        rbegin_ = new_rbegin;
        size_ --;
    }

    void pop_front()
    {
        if(size_ == 0) return;
        if(size_ == 1)
        {
            free(begin_);
            begin_ = nullptr;
            rbegin_ = nullptr;
            size_ --;
            return ;
        }
        Node<T>* new_begin = begin_->next;
        free(new_begin->prev);
        new_begin->prev = nullptr;
        begin_ = new_begin;
        size_ --;
    }

    void clear()
    {
        while (size_ > 0)
        {
            pop_back();
        }
    }

    iterator begin() { return iterator(begin_); }
    const_iterator begin() const { return const_iterator(begin_); }
    iterator end() { iterator ret(rbegin_); ret.reach_end_ = true; return ret;}
    const_iterator end() const { const_iterator ret(rbegin_); ret.reach_end_ = true; return ret;}
    T& front() { return begin_->val; }
    const T& front() const { return begin_->val; }

    T& back() { return rbegin_->val; }
    const T& back() const { return rbegin_->val; }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    iterator erase(iterator it)
    {
        iterator ret = it;
        ret++;
        if(it.p_->prev) it.p_->prev->next = it.p_->next;
        if(it.p_->next) it.p_->next->prev = it.p_->prev;
        delete it.p_;
        it.p_ = nullptr;
        return ret;
    }

    iterator insert(iterator position, const T& val)
    {
        if(position == begin())
        {
            push_front(val);
            return iterator(begin_);
        }
        if (position == end())
        {
            push_back(val);
            return iterator(rbegin_);
        }
        iterator ret;
        ret.p_ = new Node<T>(val);

        position.p_->prev->next = ret.p_;
        ret.p_->prev = position.p_->prev;

        position.p_->prev = ret.p_;
        ret.p_->next = position.p_;

        size_++;

        return ret;
    }

    ~list() { clear(); }


private:
    size_t size_ = 0;
    Node<T>* begin_ = nullptr;
    Node<T>* rbegin_ = nullptr;
};

template<typename UnaryPredicate, class InputIt>
InputIt find_if(
    InputIt first, 
    InputIt last,
    UnaryPredicate f)
{
    for(; first != last; first++)
    {
        if(f(*first)) return first;
    }

    return last;
}

} // namespace ecs





#endif // _ECS_LIST_H_
