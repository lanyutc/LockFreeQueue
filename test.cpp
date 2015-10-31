#include "lockfree_queue.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>

struct opUnit {
	string name;
	int32_t op;

	void operator = (const opUnit& v2) 
	{
		name = v2.name;
		op   = v2.op;
	}
};


LFqueue<opUnit> q;
atomic_int producerCount(0);
atomic_int consumerCount(0);
atomic_int endThreadCount(0);
atomic<bool> done (false);
const int iterations = 1000000;
const int producerThreadCount = 4;
const int consumerThreadCount = 4;


void* producer(void *)
{
	for (int i = 0; i < iterations; i++) {
		opUnit u;
		u.op = ++producerCount;
		printf("producer:%d\n",u.op);
		while (!q.push(u));
	}
	++endThreadCount;
	pthread_exit(0);
}

void* consumer(void *)
{
	sleep(1);
	opUnit u;
	while (!done) {
		while (q.pop(u)) {
			++consumerCount;
//			printf("consumer:%d\n",consumerCount.load());
			printf("consumer:%d\n",u.op);
		}
	}

	while (q.pop(u)) {
		++consumerCount;
//		printf("consumer:%d\n",consumerCount.load());
		printf("consumer:%d\n",u.op);
	}

	++endThreadCount;
	pthread_exit(0);
}

int main()
{
	if (q.isLockFree()) {
		cout << "i am lockfree" << endl;
	}

	pthread_t tid1, tid2;
	for (int i = 0; i != producerThreadCount; ++i) {
		 int r = pthread_create(&tid1, NULL, producer, NULL);
		 if (r != 0) {
			 cout << "create producer:" << r << endl;
		 }
	}

	done = true;

	for (int i = 0; i != consumerThreadCount; ++i) {
		 int r = pthread_create(&tid2, NULL, consumer, NULL);
		 if (r != 0) {
			 cout << "create consumer:" << r << endl;
		 }
	}

	while (1) {
		sleep(1);
		if (endThreadCount == (producerThreadCount + consumerThreadCount)) {
			fflush(stdout);
			break;
		}
	}

	return 0;
}
