// crl_test.cpp : Defines the entry point for the console application.
//
#include <crl/crl.h>
#include <iostream>
#include <chrono>
#include <numeric>

void testOutput(crl::queue *queue) {
	for (auto i = 0; i != 1000; ++i) {
		queue->async([i] { std::cout << "Hi from serial queue: " << i << std::endl; });
		if ((i % 100) == 99) {
			crl::async([i] { std::cout << "Hi from async: " << i << std::endl; });
			queue->sync([i] { std::cout << "Hi from queue sync: " << i << std::endl; });
		}
	}
	for (auto i = 0; i != 1000; ++i) {
		crl::async([i] { std::cout << "Hi from crazy: " << i << std::endl; });
	}
}

constexpr auto kQueueCount = 8;

struct CacheLine {
	static constexpr auto kSize = 128; // To be sure.

	int value;
	char padding[kSize - sizeof(int)];
};

int operator+(int already, const CacheLine &a) {
	return already + a.value;
}

std::atomic<int> added = 0;
int testCounting(crl::queue (&queues)[kQueueCount]) {
	CacheLine result[kQueueCount] = { 0 };
	for (auto i = 0; i != 100000; ++i) {
		for (auto j = 0; j != kQueueCount; ++j) {
			queues[j].async([&result, j] { ++result[j].value; });
		}
		if ((i % 10000) == 9999) {
			crl::async([i] { ++added; });
			const auto j = ((i + 1) / 10000) % kQueueCount;
//			queues[j].sync([j, &result] { ++result[j].value; });
		}
	}
	for (auto j = 0; j != kQueueCount; ++j) {
		queues[j].sync([&result, j] { ++result[j].value; });
	}
	return std::accumulate(std::begin(result), std::end(result), 0) + added;
}

int main() {
	crl::queue testQueue[kQueueCount];
//	testOutput(&testQueue[0]);
	for (int i = 0; i != 5; ++i) {
		auto start_time = std::chrono::high_resolution_clock::now();
		auto result = testCounting(testQueue);
		auto end_time = std::chrono::high_resolution_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		std::cout << "Time: " << ms.count() / 1000. << " (" << result << ")" << std::endl;
	}
	std::cout << "Finished." << std::endl;
	int a = 0;
	std::cin >> a;
	return 0;
}
