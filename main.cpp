#include <iostream>

#include "lru/lru.hpp"

int fibonacci(int n) {
  if (n < 2) return 1;
  return fibonacci(n - 1) + fibonacci(n - 2);
}

int fibonacci_cached(int n, LRU::Cache<int, int>& cache) {
  if (n < 2) return 1;
  if (cache.contains(n)) {
    return cache.find(n);
  }

  const auto value =
      fibonacci_cached(n - 1, cache) + fibonacci_cached(n - 2, cache);
  return cache.insert(n, value);
}

auto main() -> int {
  auto fib = LRU::memoize<int>(fibonacci);
  std::cout << fib(10) << std::endl;
  std::cout << fib.hit_rate() << std::endl;
  std::cout << fib.total_accesses() << std::endl;
  std::cout << fib.total_hits() << std::endl;

  std::cout << fib(10) << std::endl;
  std::cout << fib.hit_rate() << std::endl;
}
