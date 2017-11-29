// crl_test.cpp : Defines the entry point for the console application.
//
#include <crl/crl.h>
#include <iostream>
#include <chrono>

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

std::atomic<int> added = 0;
int testCounting(crl::queue *queue) {
	auto result = 0;
	for (auto i = 0; i != 10000000; ++i) {
		queue->async([i, &result] { ++result; });
		if ((i % 10000) == 9999) {
			crl::async([i] { ++added; });
			queue->sync([i, &result] { ++result; });
		}
	}
	return result + added;
}

int main() {
	crl::queue testQueue;
	testOutput(&testQueue);
	std::cout << "Hello, World!" << std::endl;
//	for (int i = 0; i != 5; ++i) {
//		auto start_time = std::chrono::high_resolution_clock::now();
//		auto result = testCounting(&testQueue);
//		auto end_time = std::chrono::high_resolution_clock::now();
//		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
//		std::cout << "Time: " << ms.count() / 1000. << " (" << result << ")" << std::endl;
//	}
	int a = 0;
	std::cin >> a;
	return 0;
}
