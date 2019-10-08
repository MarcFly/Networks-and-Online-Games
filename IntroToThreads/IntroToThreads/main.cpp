#include <thread>
#include <Windows.h>
#include <stdlib.h>
#include <iostream>
#include <mutex>

void funct(int arg1, int arg2, int arg3)
{
	Sleep(1000);
	printf("Hello Thread");
	std::cout << arg1 << arg2 << arg3;
}

int counter = 0;
std::mutex mtx;
void increment(int iterations) {
	std::unique_lock<std::mutex> lock(mtx);
	lock.unlock();

	for (int i = 0; i < iterations; ++i) {
		lock.lock();
		counter++;
		lock.release();
	}
}
int main()
{
	std::thread t = std::thread(funct, 1, 2, 3);
	
	while (true)
	{
		for(int i = 0; i < 10000; ++i)
		{ }
		break;
	}
	
	t.join(); // Wait for thread

	std::thread t1(increment, 100000);
	std::thread t2(increment, 100000);
	t1.join();
	t2.join();

	std::cout << "Counter = " << counter << std::endl;

	system("pause");

	return 0;
}


{
	Task* task = nullptr;
	std::unique_lock<std::mutex> lock(mutex); // Lock the queue check
	while (task == nullptr) // mientras no hay tasks aañadida
	{
		if (tasks.empty())
		{
			cond.wait(lock); // wait for the condition variable to change
			// While it is locked, other mutexes are unlocked, so they can add things

		}
		else {
			task = sheduledTasks.pop_front();
		}
	}


}