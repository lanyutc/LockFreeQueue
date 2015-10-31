#ifndef _LOCKFREE_QUEUE_
#define _LOCKFREE_QUEUE_

#include <atomic>
#include <map>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

using namespace std;

template <typename T>
class LFqueue
{
public:
	LFqueue()
	{
		node *n = new node();
		head_.store((unsigned long)n, memory_order_relaxed);
		tail_.store((unsigned long)n, memory_order_release);
	}

	struct node {
		T data;
		atomic_ulong next;

		node(T const &v) : data(v)
		{
			next.store((unsigned long)0, memory_order_release);
		}

		node()
		{
		}
	};


public:
	bool isLockFree() const
	{
		return head_.is_lock_free() && tail_.is_lock_free();
	}

	bool push(T const &t)
	{
		node *n = new node(t);

		if (!n) return false;
		
		for (;;) {
			
			unsigned long tail = tail_.load(memory_order_acquire);

			node *tNext   = nextNodePtr(tail_);
			node *tailPtr = nodePtr(tail_);

			unsigned long tail2= tail_.load(memory_order_acquire);
			if (likely(tail == tail2)) {

				if (tNext == 0) {
					unsigned long next = 0;
					if (tailPtr->next.compare_exchange_weak(next, (unsigned long)n)) {
						tail_.compare_exchange_strong(tail, (unsigned long)n);
						return true;
					}
				} else {
					tail_.compare_exchange_strong(tail, (unsigned long)tNext);
				}

			}	
		}
	}

	bool pop(T &ret)
	{
		for (;;) {
			unsigned long head = head_.load(memory_order_acquire);
			unsigned long tail = tail_.load(memory_order_acquire);

			node *hNext   = nextNodePtr(head_);
			node *headPtr = nodePtr(head_);

			unsigned long head2= head_.load(memory_order_acquire);
			if (likely(head == head2)) {

				if (head == tail) {
					if (hNext == 0) return false;

					tail_.compare_exchange_strong(tail, (unsigned long)hNext);
				} else {
					if (hNext == 0) continue;

					ret = hNext->data;
					if (head_.compare_exchange_weak(head, (unsigned long)hNext)) {
						delete headPtr;
						return true;
					}
				}

			}
		}
	}

private:
	node *nodePtr(const atomic_ulong &a)
	{
		return reinterpret_cast<node *>(a.load(memory_order_acquire));
	}

	node *nextNodePtr(const atomic_ulong &a)
	{
		node *n = nodePtr(a);
		if (n) {
			return nodePtr(n->next);
		}
		return 0;
	}

private:
	atomic_ulong head_;
	atomic_ulong tail_;
};



#endif
