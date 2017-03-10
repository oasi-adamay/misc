#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

#if 0
int main(void) {
	for (int i = 0; i < 10; i++) {
		std::cout << i << std::endl;
	}
	return 0;
}
#endif



int main(void) {
	std::mutex theMtx;

	std::vector<std::thread> threads;

	int num_thread = 4;
	for (int i = 0; i < num_thread; i++) {
		int num = 10 / num_thread;
		int s = num * i;
		if (i - 1 == 10 / num_thread) num += 10 % num_thread;

		threads.push_back(
			std::thread([&theMtx](int s,int num) {
				std::lock_guard<std::mutex> lock(theMtx);
				for (int i = s; i < s + num; i++) {
					std::cout << i << std::endl;
				}
			}, s, num)
		);
	}


	for (auto iter = threads.begin(); iter != threads.end(); ++iter) {
		iter->join();
	}

	return 0;
}

