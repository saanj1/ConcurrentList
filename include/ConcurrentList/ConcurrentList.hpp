#pragma once 
/**
 * @file ConcurrentList.hpp
 * @author Sanjeev Kumar M
 * @date 28/11/2020 7:30 PM
 * 
 * This module contains the implementation
 * for ConcurrentList::ConcurrentList class
 */


#define SNJ_MAKE_NONCOPYABLE(c)\
private:                       \
  c(const c&) noexcept = delete;\
  c& operator=(const c&) noexcept = delete

#define SNJ_MAKE_NONMOVABLE(c)\
private:                      \
  c(c&&) noexcept = delete;   \
  c& operator=(c&&) noexcept = delete

#include <memory>
#include <atomic>
#include <utility>

namespace ConcurrentList {
  template <typename ValueType, 
            typename Allocator = std::allocator<ValueType>>
  class ConcurrentList {
   public:
    /**
     * This structure used to construct linkedlist
     * instance of this struct will hold data and 
     * pointer to the the next node
     */
    struct ListNode {
      ValueType data;
      std::atomic<ListNode*> next;
      ListNode(const ValueType& __data) {
        this->data = __data;
        this->next = nullptr;
      }
    };

    struct iterator {
      //typdefs 
      using value_type = ValueType;
      using pointer = ValueType*;
      using reference = ValueType&;
      using iterator_category = std::random_access_iterator_tag;

      //This is destructor
      ~iterator() = default;

      /**
       * This is copy constructor
       * 
       * @param[in]
       *    __iter - lvalue reference of iterator
       */
      iterator(const iterator& __iter) noexcept {
        node_ptr = __iter.get_node_ptr();
      } 

      /**
       * This is move constructor
       * 
       * @param[in]
       *    __iter - rvalue reference of the iterator
       */
      iterator(iterator&& __iter) noexcept {
        node_ptr = __iter.node_ptr;
        __iter.node_ptr = nullptr;
      }

      /**
       * This is copy assignment operator
       * 
       * @param[in]
       *    __iter - lvalue reference of the iterator
       * 
       * @return
       *   lvalue reference to this
       */
      iterator& operator=(const iterator& __iter) noexcept {
        node_ptr = __iter.node_ptr;
        return *this;
      }

      /**
       * This is move assignment operator
       * 
       * @param[in]
       *    __iter - rvalue reference of the interator
       * 
       * @return 
       *    lvalue reference to this
       */
      iterator& operator=(iterator&& __iter) noexcept {
        node_ptr = __iter.node_ptr;
        __iter.node_ptr = nullptr;
        return *this;
      }

      //This is the default constructor
      iterator()
        : node_ptr(nullptr)
      {}

      //This is the constructor taking nullptr
      explicit iterator(std::nullptr_t) 
        : node_ptr(nullptr)
      {}

      /**
       * This is the constructor with ptr to the list 
       * node to initialize
       *
       * @param[in]
       *    __node_ptr - pointer to the listnode
       */
      explicit iterator(ListNode* __node_ptr)
        : node_ptr(__node_ptr)
      {}

      /**
       * This is dereference operator
       * 
       * @return 
       *    reference to the value
       */
      reference operator*() {
        return (node_ptr->data);
      }

      
      /**
       * This is post increment operator
       */
      iterator& operator++(int) {
        iterator copy(*this);
        node_ptr = node_ptr->next.load();
        return copy;
      }

      /**
       * This is preincrement operator
       */
      iterator& operator++() {
        node_ptr = node_ptr->next.load();
        return *this;
      }

      /**
       * This is increment operator
       */
      iterator& operator+(int __n) {
        int node_count = 0;
        while(node_count < __n) {
          node_ptr = node_ptr->next.load();
          ++node_count;
        }
        return *this;
      }

      /**
       * This is equality operator
       */
      bool operator==(const iterator& __iter) {
        return node_ptr == __iter.node_ptr;
      }

      /**
       * This is not equality operator
       */
      bool operator!=(const iterator& __iter) {
        return node_ptr != __iter.node_ptr;
      }

      /**
       * This stores the pointer to the node
       */
      ListNode* node_ptr;
    };
    using AllocatorTraits = std::allocator_traits<Allocator>;
    template<typename T>
    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
   public:
    //Life cycle management
    ~ConcurrentList() = default;
    //Implemet these if require, disabling now
    SNJ_MAKE_NONCOPYABLE(ConcurrentList);
    SNJ_MAKE_NONMOVABLE(ConcurrentList);

    //This is the constructor 
    ConcurrentList()
      : _m_head(nullptr),
        _m_node_allocator()
    {}

    /**
     * This function used to push a new element to the list
     * 
     * @param[in]
     *    __data - lvalue reference to the data 
     */
    void push(ValueType& __data) {
      _m_push(__data);
    }

    /**
     * This function used to push a new element to the list
     * 
     * @param[in]
     *    __data - rvalue reference to the data 
     */
    void push(ValueType&& __data) {
      _m_push(std::move(__data));
    }

    /**
     * This function used to push new element after a specified
     * element
     * 
     * @param[in]
     *      __n_data - element after which new element to be pushed
     * 
     *      __data - lvalue reference data to be pushed to the queue
     * 
     * @return
     *      bool - true indicating push was successfull, false indicating
     *            fail to find the specified node, couldn't push
     */
    bool push(const ValueType& __n_data, ValueType& __data) {
      return _m_n_push(__n_data, __data);
    }

    /**
     * This function used to push a new element after a specified element
     * 
     * @param[in] 
     *    __n_data - element after which new element to be pushed
     *  
     *    __data - rvalue reference to the data to be pushed to the  queue
     * 
     * @return 
     *    bool - true indicating push was successfull, false indicating 
     *          fail to find the specified node, push failed.
     */
    bool push(const ValueType& __n_data, ValueType&& __data) {
      return _m_n_push(__n_data, std::move(__data));
    }

    /**
     * This function returns the new iterator to the head of the list
     *
     * @return
     *    iterator instance with head of the list
     */
    iterator begin() {
      return iterator(_m_head);
    }

    /**
     * This function returns the iterator with nullptr
     * @return
     *    iterator instance with nullptr
     */
    iterator end() {
      return iterator(nullptr);
    }

   private:
    template<typename... Args>
    void _m_push(Args&&... __args) {
      auto new_node = _m_create_node(std::forward<Args>(__args)...);
      auto curr_head = _m_head.load();
      new_node->next = curr_head;
      while(!_m_head.compare_exchange_weak(curr_head, new_node, 
                              std::memory_order_release)) {
          curr_head = _m_head.load();
          new_node->next = curr_head;
      }
    }

    template<typename... Args>
    bool _m_n_push(const ValueType &__n_data, 
                            Args&&... __args) {
      auto curr = _m_head.load();
      while(curr != nullptr && curr->data != __n_data) {
        curr = curr->next->load();
      }
      if (curr == nullptr) return false;
      auto new_node = _m_create_node(std::forward<Args>(__args)...);
      auto curr_next = curr->next.load();
      new_node->next = curr_next;
      while(!curr->next.compare_exchange_weak(curr_next, new_node, 
                std::memory_order_release)) {
        curr_next = curr->next.load();
        new_node->next = curr_next;
      }
      return true;
    }

    template<typename... Args> 
    ListNode* _m_create_node(Args&&... __args) {
      auto new_node_ptr = _m_node_allocator.allocate(1);
      _m_node_allocator.construct(new_node_ptr, std::forward<Args>(__args)...);
      return new_node_ptr;
    }

    /**
     * This member holds the current head of the list
     */
    std::atomic<ListNode*> _m_head;

    /**
     * This is the node allocator which allocates memory
     * for new nodes
     */
    NodeAllocator<ListNode> _m_node_allocator;
  };
}