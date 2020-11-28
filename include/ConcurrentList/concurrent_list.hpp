#pragma once 
/**
 * This module contains the implementation
 * for DS::ConcurrentList class
 */

#include <memory>
#include <atomic>
#include <utility>

namespace ConcurentList {
	template<typename T>
	class ConcurrentList {
	 public:
		//typedefs 
		typedef T value_type; 
		/**
		 * This structure used to construct linkedlist, 
		 * this will hold data and pointer to the next element
		 */
		struct ListNode {
			value_type value;
			std::atomic<ListNode*> next;
			ListNode(value_type __value) {
				this->value = __value;
				this->next = nullptr;
			}
		};
	
	/**
	 * This is the iterator class
	 */
	class iterator {
		 public:
				//typedefs 
			typedef iterator self_type;
			typedef value_type* pointer;
			typedef value_type& reference;

			/**
			 * This is the constructor
			 */
			iterator(ListNode* __ptr) : _m_ptr(__ptr) {}

			pointer operator->() { return this->_m_ptr; }

			reference operator*() { return this->_m_ptr->value; }

			self_type operator++() { this->_m_ptr = _m_ptr->next.load();return *this; }

			self_type operator++(int){
				self_type t_iter = *this;
				this->_m_ptr = this->_m_ptr->next.load();
				return t_iter;
			}

			bool operator==(self_type& __rhs) { return this->_m_ptr == __rhs._m_ptr; }

			bool operator!=(self_type&& __rhs) { return this->_m_ptr != __rhs._m_ptr; }

		 private:
		 	ListNode *_m_ptr; 
		};

	//Life cycle management
		~ConcurrentList() {
			ListNode *t_curr = this->_m_head.load(), *t_next;
			while(t_curr != nullptr) {
				t_next = t_curr->next.load();
				delete t_curr;
				t_curr = t_next;
			}
		}
		//Implement these if required, disabling now
		ConcurrentList(const ConcurrentList& __list) noexcept = delete;
		ConcurrentList(ConcurrentList&& __list) noexcept = delete; 
		ConcurrentList& operator=(const ConcurrentList&) noexcept = delete;
		ConcurrentList& operator=(ConcurrentList&&) noexcept = delete;
		 
	public:
		//This is the constructor
		ConcurrentList()
			: _m_head(nullptr)
		{}
		
		template<typename... Args>
		void _m_push(Args&&... __args) {
			ListNode *new_node = _m_new_listnode(std::forward<Args>(__args)...);
			ListNode* old_head = this->_m_head.load();
			new_node->next.store(old_head);
			while(false == this->_m_head.compare_exchange_weak(old_head, 
							new_node, std::memory_order_release)) {
				new_node->next.store(old_head);   
			}
		}
			
		/**
		 * This function pushes new element at the head of the list
		 *
		 * @param[in] 
     *      __val - lvalue reference to the value 
     */	
		void push(value_type& __val) {
			this->_m_push(__val);
		}
		
		/**
		 * This function pushes new value at the head of the list
		 *
		 * @param[in] 
     *      __val - rvalue reference to the value
     */
		void push(value_type&& __val) {
			this->_m_push(std::move(__val));
		}

		template<typename... Args>
    bool _m_push_next(value_type &__n_val, Args&&... __args) {
			ListNode* t_curr = this->_m_head.load();
			while(t_curr != nullptr && t_curr->value != __n_val) {
				t_curr = t_curr->next.load();
			}
			if (t_curr == nullptr) return false;
			ListNode* t_old_next = t_curr->next.load();
			ListNode* t_new_node = _m_new_listnode(std::forward<Args>(__args)...);
			t_new_node->next.store(t_old_next);
			while(false == t_curr->next.compare_exchange_weak(t_old_next, 
				t_new_node, std::memory_order_release)) {
				t_new_node->next.store(t_old_next);
			}
			return true;
		}

    /**
     * This function pushes new element after a specified element
     * @param[in]
     *      __n_val - value of the node after which new element to
     * 						be inserted
     *      __val - lvalue reference to of the value
     * @return
     *      boolean value - True indicating we could find the 
     *      given node and push  new node to the next of that node
     */   
		bool push(value_type& t_n_val, value_type& t_val) {
			return this->_m_push_next(t_n_val, t_val);
		}
    
		/**
     * This function pushes new element after a specified element
     * @param[in]
     *      __n_val - value of the node after which new element to
     * 						be inserted
     *      __val - rvalue reference to of the value
     * @return
     *      boolean value - True indicating we could find the 
     *      given node and push  new node to the next of that node
     */   
    bool push(value_type& t_n_val, value_type&& t_val) {
			return this->_m_push_next(t_n_val, std::move(t_val));
		}

		/**
		 * This function returns the current head pointer
		 * 
		 * @Return
		 * 			pointer to the head node
		 */
		ListNode* get_head() {
			return this->_m_head.load();
		}

		/**
		 * This function returns the begin iterator
		 */
		iterator begin() {
			return iterator(this->_m_head.load());
		}

		/**
		 * This function returns the end iterator
		 * TODO: define a constant end iterator, this 
     * doesn't work always
     */
		iterator end() {
			return iterator(nullptr);
		}

	 private:
		/**
		* This function allocates a new listnode with the given data
		*/
		template<typename... Args>
		static ListNode* _m_new_listnode(Args&&... args) {
			return new ListNode(std::forward<Args>(args)...);
		}
		
		/**
		* This stores the head of the list
		*/
		std::atomic<ListNode*> _m_head;
	};
}
