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
		threads.push_back(
			std::thread([&theMtx](int i) {
				std::lock_guard<std::mutex> lock(theMtx);
				std::cout << i << std::endl;
			}, i)
		);
	}


	for (auto iter = threads.begin(); iter != threads.end(); ++iter) {
		iter->join();
	}

	return 0;
}

