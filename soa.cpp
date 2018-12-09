#include "soarray.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <array>
#include <random>
using namespace std::chrono;

struct A {
  std::array<int, 256> a;
  int b;
  const char* c;
};

template<>
struct FieldCount<A> {
  const static std::size_t value = 3;
};

template<>
struct Field<A, 0> : public FieldBase<std::array<int, 256>, A, &A::a> {};

template<>
struct Field<A, 1> : public FieldBase<int, A, &A::b> {};

template<>
struct Field<A, 2> : public FieldBase<const char*, A, &A::c> {};

int main() {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(0, 100);

  // Example: Enumerating the second field of a long list of large objects.
  {
    SoArray<std::vector, A> arr;
    const long N = 1000;
    for(int i = 1; i <= N; ++i) {
      arr.push_back(A{std::array<int, 256>(), dist(mt), "abc"});
    }
    auto before = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
    long sum = 0;
    arr.iterate<1>([&](int n){
      sum += n;
    });
    auto after = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
    printf("%ld\n", sum);
    std::cout << after - before << std::endl;
  }

  // For comparison: The same code using a std::vector of large objects.
  {
    std::vector<A> arr;
    const long N = 1000;
    for(int i = 1; i <= N; ++i) {
      arr.push_back(A{std::array<int, 256>(), dist(mt), "abc"});
    }
    auto before = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
    long sum = 0;
    for(const auto& a : arr) {
      sum += a.b;
    }
    auto after = duration_cast< milliseconds >(system_clock::now().time_since_epoch()).count();
    printf("%ld\n", sum);
    std::cout << after - before << std::endl;
  }

  return 0;
}
