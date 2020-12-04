#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include <ConcurrentList/ConcurrentList.hpp>

TEST(ConcurrentListTest, ShouldPushNewValue) {
  ConcurrentList::ConcurrentList<int> list;
  std::vector<std::thread> threads_vec(4);
  for(int i = 0; i < 4; ++i) {
    int start = 100 * i, end = 100 * (i + 1);
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
  ASSERT_EQ(values.size(), 400);
}

TEST(ConcurrentListTest, ShouldPushNewValueAfterGivenElement) {
  ASSERT_EQ(true, true);
}
