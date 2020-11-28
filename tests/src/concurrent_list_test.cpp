#include <iostream>
#include <thread>
#include <algorithm>
#include <vector>
#include <ConcurrentList/concurrent_list.hpp>
#include <cassert>

#define assertm(exp, msg) assert(((void)msg, exp))

void test_push_value(int thread_count, int values_to_push) {
  ConcurentList::ConcurrentList<int> list;
  std::vector<std::thread> threads_vec(thread_count);
  for(int i = 0; i < thread_count; ++i) {
    int start = values_to_push * i, end = values_to_push * (i+1);
    threads_vec[i] = std::thread([&list = list, start, end]() {
      for(int i = start; i < end; ++i) list.push(i);
    });
  }
  for(auto &t : threads_vec) {
    t.join();
  }
  std::vector<int> values;
  for(auto it = list.begin(); it != list.end(); ++it) {
    values.push_back(*it);
  }
  assertm(values.size() == thread_count * values_to_push, 
      "PushAtHead: All elements present in the list");
  std::sort(values.begin(), values.end());
  for(int i = 0; i < values.size(); ++i) {
    assertm(i == values[i], "PushAtHead: All elements present in the list");
  }
  std::cout << "Test PushVaues : Passed..." << std::endl;
}

void test_push_value_after_n(int num_threads, int num_values) {
  ConcurentList::ConcurrentList<int> list;
  std::vector<std::thread> threads_vec(num_threads);
  for(int i =0; i <num_threads; ++i) {
    int val = i, start = num_values * i + num_threads, end = num_values * (i+1) + num_threads;
    list.push(val);
    threads_vec[i] = std::thread([&list = list, val=val, 
            start=start, end=end]() {
      int t_val = val;
      for(int i =start; i < end; ++i) {
        list.push(t_val, i);
      }
    });
  }
  for(auto &t : threads_vec) {
    t.join();
  }
  for(int value = 0; value < num_threads; ++value) {
    auto it = list.begin();
    while(it != list.end() && *it != value) ++it;
    assertm(it != list.end(), "PushAfterN: All elements present after N");
    std::vector<int> values;
    int start = num_threads + (value * num_values), end = num_threads + (value + 1) * num_values;
    for(;it != list.end();++it) {
      if (*it >= start && *it < end) {
        values.push_back(*it);
      }
    }
    assertm(values.size() == num_values, "PushAfterN: All values are not present");
    std::sort(values.begin(), values.end());
    for(int i = 0, t_v = start; t_v < end; i++, t_v++) {
      assertm(t_v == values[i], "PushAfterN: All values are not present");  
    }
  }
  std::cout << "Test PushAfterN : Passed..." << std::endl;
}

int main() {
  test_push_value(4, 100);
  test_push_value_after_n(4, 100);
  return 0; 
}
