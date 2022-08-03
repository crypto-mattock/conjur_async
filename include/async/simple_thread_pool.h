#pragma once

#include <async/generic_thread_pool.h>
#include <async/simple_thread_pool_interface.h>
#include <queue>

namespace Conjur { namespace Async {

  /**
   * @brief Implementation of a simple thread pool.
   * @tparam ThreadT thread wrapper implementation
   */
  template <typename ThreadT = SimpleThreadWrapper>
  class SimpleThreadPool final
    : public GenericThreadPool<SimpleThreadPoolInterface, ThreadT>
  {
  public:

    /**
     * @brief Constructs a new simple thread pool.
     * @param poolSize number of threads
     */
    SimpleThreadPool(size_t poolSize);

    virtual ~SimpleThreadPool() = default;

    /**
     * @brief Posts a task to the thread pool.
     * @param work task
     */
    virtual void Post(ThreadPoolInterface::Work&& work) override;

  protected:

    /**
     * @brief Pops a task from the queue.
     * @return task
     */
    virtual ThreadPoolInterface::Work PopNextTask() override;

  private:

    using TaskCollection = std::queue<ThreadPoolInterface::Work>;

    TaskCollection Tasks;
    std::mutex TasksMutex;
  };

  template <typename ThreadT>
  inline SimpleThreadPool<ThreadT>::SimpleThreadPool(size_t poolSize)
    : GenericThreadPool<SimpleThreadPoolInterface, ThreadT>{
      poolSize,
      [] (WorkerT worker)
      {
        return ThreadT{worker};
      }}
  {
  }

  template <typename ThreadT>
  inline void SimpleThreadPool<ThreadT>::Post(ThreadPoolInterface::Work&& work)
  {
    std::lock_guard<std::mutex> guard{TasksMutex};

    Tasks.push(std::move(work));
    NotifyNewTask();
  }

  template <typename ThreadT>
  inline ThreadPoolInterface::Work SimpleThreadPool<ThreadT>::PopNextTask()
  {
    std::lock_guard<std::mutex> guard{TasksMutex};

    ThreadPoolInterface::Work work;

    if (!Tasks.empty())
    {
      work = std::move(Tasks.front());
      Tasks.pop();
    }

    return work;
  }

} }
