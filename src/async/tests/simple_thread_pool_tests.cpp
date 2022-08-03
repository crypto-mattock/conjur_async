#include <async/simple_thread_pool.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <future>
#include <memory>

namespace Conjur { namespace Async { namespace Tests {

  class SimpleThreadPoolTests
    : public ::testing::Test
  {
  };

  TEST_F(SimpleThreadPoolTests, TestEnqueue)
  {
    std::vector<::testing::MockFunction<void(void)>> tasks(32);
    std::vector<std::promise<void>> promises(tasks.size());
    std::vector<std::future<void>> futures;

    for (auto i = 0; i != promises.size(); ++i)
      futures.emplace_back(std::move(promises[i].get_future()));

    for (auto i = 0; i != tasks.size(); ++i)
      EXPECT_CALL(tasks[i], Call()).Times(1);

    {
      std::unique_ptr<SimpleThreadPoolInterface> executor = std::make_unique<SimpleThreadPool<>>(4);
      executor->Start();

      for (auto i = 0; i != tasks.size(); ++i)
      {
        executor->Post(
          [i, &task = tasks[i], &promise = std::move(promises[i])] ()
          {
            auto& work = task.AsStdFunction();
            work();

            promise.set_value();
          });
      }

      for (auto i = 0; i != futures.size(); ++i)
        futures[i].get();

      executor->Stop();
    }
  }

} } }
