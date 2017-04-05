#pragma once
#define DBJVERSION __DATE__ __TIME__
#pragma message( "Compiling: " __FILE__ ", Version: " DBJVERSION)
#pragma comment( user, "(c) 2017 by dbj@dbj.org code, Version: " DBJVERSION ) 
/*
Copyright 2017 dbj@dbj.org

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include <iostream>
#include <type_traits>
#include <utility>
#include <memory>

namespace dbj {
	namespace tree {
		/* 
		Binary Tree made more useful the easy and quick way thanks to modern C++

		(c) 2017 by dbj@dbj.org
		*/

		/*		
		T is data type stored		
		It has to be deletable, swappable and streamable for output


		tree imp operates with naked pointers
		that makes it more obvious what is goin on
		also the default constructor is not allowed
		that makes no sense and will introduce an 'empty BinaryNode' singularity
		*/
		template<
			typename T
		>
		class BinaryNode {
		public:
			typedef BinaryNode *	    NodePointer;
			typedef T					DataType;
			typedef T *					DataPointer;
			typedef BinaryNode<T>		BinaryNodeType;
			typedef std::unique_ptr<BinaryNode> Guardian;
			/*
			each BinaryNode processing function has to conform to this footprint
			tree traversal stops on false returned
			*/
			typedef bool(*BinaryNodeVisitorType) (NodePointer);
		private:
			DataPointer					data_;
			NodePointer					left_;
			NodePointer					right_;
		public:
			/*
			Factory method.
			Following the http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2006/n2027.html#Perfect_Forwarding

			TODO:users should make_node_pointer only once when "starting" a tree,
			keeping the root node in std::unique_ptr
			*/
			static __forceinline
				NodePointer make_node_pointer(T && root_data) {
				return new BinaryNode(std::forward<T>(root_data));
			}
			static	__forceinline 
				DataPointer make_data_pointer( const T & data_ ) {
				return new T(data_);
				// return std::allocator<T>().allocate(sizeof(T));
			}
			
			/* default ctor */
			BinaryNode() 
				: data_(nullptr),
			      left_(nullptr), 
				  right_(nullptr)
			{
			}
			/*
			reminder: delete will call BinaryNode destructor on 
			left and right 
			thus this destructor will delete the whole subtree 
			under this instance
			*/
			~BinaryNode() {
				DBJ_VERIFY(this);
				if (data_)
					delete data_; data_ = nullptr;
				if (left_)
					delete left_; left_ = nullptr;
				if (right_)
					delete right_; right_ = nullptr;
			}

			/* construct with data */
			BinaryNode( T && ndata_ ) 
				: left_  (),
				  right_ ()
			{
				data_ = make_data_pointer( std::forward<T>(ndata_));
			}

			/* copying */
			BinaryNode(const BinaryNode & other_)
			{
				DBJ_VERIFY(!data_);
				this->data_ = other_.data_;
				DBJ_VERIFY(!left_);
				this->left_ = other_.left_;
				DBJ_VERIFY(!right_);
				this->right_ = other_.right_;
			}
			/*  */
			const BinaryNode &  operator = (const BinaryNode & other_) {
				if (this == &other_) return;
				if (data_) delete data_;
					this->data_  = other_.data_  ;
					if (left_) delete left_;
					this->left_  = other_.left_;
					if (right_) delete right_;
					this->right_ = other_.right_;
					return *this;
				}

			/* Moving */
			BinaryNode(BinaryNode && other_) 
			{
				swap(*this, other_);
			}

			BinaryNode & operator = (BinaryNode && other_)
			{
				if (this != &other_ )
					swap(*this, other_);
				return *this;
			}

			/*
			the C++11 std::swap, requires the type T to be MoveConstructible and MoveAssignable
			note: always use this idiom to overload std::swap for a class/struct
			*/
			friend void swap(BinaryNode & left_, BinaryNode & right_) 
			{
				if (left_ == right_) return;
				std::swap(left_.left_ ,right_.left_ );
				std::swap(left_.right_ , right_.right_);
				std::swap(left_.data_, right_.data_);
			}

			const bool operator == (const BinaryNode & other_) const {
				if ( this->data_ == other_.data_ )
					if (this->left_ == other_.left_)
						if (this->right_ == other_.right_)
							return true;
				return false;
			}

			const bool operator != (const BinaryNode & other_) const {
				return !(*this == other_);
			}

			const DataPointer  data()  const noexcept {  return this->data_;  }
			const NodePointer  left () const noexcept {  return this->left_;  }
			const NodePointer  right() const noexcept {  return this->right_; }

			/*
			Variadic insert
			*/
			template < typename ... Args >
			const void insert(Args&&... args)
			{
				const size_t size = sizeof...(args);
				T arglist[size] = { args... };
				for (auto & data_ : arglist) {
					(void)inserter(this, data_ );
				}
			}

			/* Streaming insert */
			BinaryNode & operator << (const T & ndata) {
				inserter(this,ndata);
				return const_cast<BinaryNode &>(*this);
			}

			/* methods bellow might be public but that will confuse users */
		private:
			/*
			insertion logic is: 'store left if smaller right if not'
			insertion criteria is defined by using '<=' operator on data
			this is e.g. 0,1,2,3,4 is repeatedly give the resulting tree will
			have them all in just "right" Nodes

			BinaryNode insertion method creates balanced tree by default
			it uses operator '<=' on the data type used and stored
			it also uses BinaryNode<T>::BinaryNode_creator() to make_node_pointer the BinaryNode
			*/
			NodePointer inserter(NodePointer root, T ndata) {
				if (!root) {
					root = make_node_pointer( std::forward<T>(ndata) );
				}
				else if (
					root->data_ 
						?  ndata <= *(root->data_) 
						: (DBJ_VERIFY(root->data_),false)
					)
					root->left_ = inserter(root->left_, ndata) ;
				else
					root->right_ = 	inserter(root->right_, ndata );

				return root ;
			}
		};

		
		//visit BinaryNodes in preorder
		template<typename PT, typename F>
		static __forceinline
		void preorder(PT root_, F f) {
			if (root_ == nullptr) return;
			if (!f(root_)) return; // process
			preorder(root_->left(), f);     // left subtree
			preorder(root_->right(), f);    // right subtree
		}
		//visit BinaryNodes in inorder
		template<typename PT, typename F>
		static __forceinline
			void inorder(PT root, F f) {
			if (root == nullptr) return;
			inorder(root->left(), f);       // left subtree
			if (!f(root)) return ; // process
			inorder(root->right(), f);      // right subtree
		}

		//visit BinaryNodes in postorder
		template<typename PT, typename F>
		static __forceinline
			void postorder(PT root, F f) {
			if (root == nullptr) return;
			postorder(root->left(),  f);   // left subtree
			postorder(root->right(), f);   // right subtree
			if (!f(root)) return ; // process
		}

		/*
		Test on an example tree of BinaryNode<wstring>
		*/
		static __forceinline
			void binary_node_test(std::wostream & wos)
		{
			using std::wostream;
			const wchar_t & nl = { L'\n' };

			/*
			this lambda is our processor
			it can be any lambda, function or functor that receives pointer 
			to BinaryNode<> instance and returns bool.
			false	retval from the processor will stop the processing.
			*/
			auto printer = [](auto x) {
				DBJ_VERIFY(x);
				DBJ_VERIFY(x->data());
				std::wcout << L"[" << std::setw(9) << *(x->data()) << L"]"; return true;
			};
			/*
			An example of what some are calling 'comfortable API'
			*/
			{
				using std::wstring;
				typedef BinaryNode<wstring> BinaryNodeT;
				auto guard1 = BinaryNodeT::Guardian(BinaryNodeT::make_node_pointer(L"ROOT"));
				auto root1 = guard1.get();
				(*root1) << L"Minnie" << L"Moonnie" << L"Tiny" << L"Shiny" << L"Galaxy";
				root1->insert(L"And", L"Variadic", L"Insert", L"Also", L"Available");
				wos << nl << nl;
				dbj::treeprint::binary(wos, root1);
				wos << nl << nl;
			}
			typedef BinaryNode<int> BinaryNodeT;
			auto guard = BinaryNodeT::Guardian(BinaryNodeT::make_node_pointer(0));
			auto root = guard.get();

			(*root) << 1 << 2 << 3 << 4 << 5;

			root->insert(10, 9, 8, 7, 6);
#if 0
			wos << nl << L"Process then visit left and right: ";
			preorder(root , printer);
			wos << nl;
			wos << nl << L"Visit left, Process then  visit right: ";
			inorder(root, printer);
			wos << nl;

			wos << nl << "LVisit left, visit right then Process: ";
			postorder(root, printer);
			wos << nl;
#endif
			wos << nl << nl ;
			dbj::treeprint::binary(wos,root);
			wos << nl << nl;
		}
	} // tree
} // dbj