#pragma once

#include <async/simple_thread_wrapper.h>
#include <async/thread_pool_interface.h>
#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>

namespace Conjur { namespace Async {

  /**
   * @brief Implementation of an executor which manages a pool of threads.
   * @tparam ThreadPoolT thread pool implementation
   * @tparam ThreadT thread wrapper implementation
   */
  template <typename ThreadPoolT = ThreadPoolInterface, typename ThreadT = SimpleThreadWrapper>
  class GenericThreadPool
    : public ThreadPoolT
  {
  public:

    GenericThreadPool(const GenericThreadPool&) = delete;

    GenericThreadPool(GenericThreadPool&&) = default;

    using WorkerT = std::function<void(void)>;
    using ThreadFactoryT = std::function<ThreadT(WorkerT)>;

    /**
     * @brief Constructs a new generic thread pool.
     * @param poolSize number of threads
     * @param threadFactory factory function for single thread
     */
    GenericThreadPool(
      size_t poolSize,
      ThreadFactoryT threadFactory);

    virtual ~GenericThreadPool();

    /**
     * @brief Constructs the threads and starts the thread pool.
     */
    virtual void Start() override;

    /**
     * @brief Stops the thread pool and joins all threads.
     */
    virtual void Stop() override;

  protected:

    /**
     * @brief Wakes a thread to process new tasks. Must be invoked after a task post.
     */
    void NotifyNewTask();

    /**
     * @brief Pops a task from the queue.
     * @return task
     */
    virtual ThreadPoolInterface::Work PopNextTask() = 0;

  private:

    using ThreadCollection = std::list<ThreadT>;

    /**
     * @brief Worker function.
     */
    void Worker();

    size_t PoolSize;
    ThreadFactoryT ThreadFactory;
    std::once_flag InitOnce;
    std::once_flag DeinitOnce;
    std::atomic_bool IsStopping{false};
    std::condition_variable Awaiting;
    std::mutex AwaitingMutex;
    ThreadCollection Threads;
  };

  template <typename ThreadPoolT, typename ThreadT>
  inline GenericThreadPool<ThreadPoolT, ThreadT>::GenericThreadPool(
    size_t poolSize,
    ThreadFactoryT threadFactory)
    : PoolSize{poolSize}
    , ThreadFactory{std::move(threadFactory)}
  {
  }

  template <typename ThreadPoolT, typename ThreadT>
  inline GenericThreadPool<ThreadPoolT, ThreadT>::~GenericThreadPool()
  {
    Stop();
  }

  template <typename ThreadPoolT, typename ThreadT>
  inline void GenericThreadPool<ThreadPoolT, ThreadT>::Start()
  {
    std::call_once(
      InitOnce,
      [this] ()
      {
        for (size_t i = 0; i != PoolSize; ++i)
        {
          Threads.emplace_back(
            ThreadFactory(
              [this] ()
              {
                Worker();
              }));
        }
      });
  }

  template <typename ThreadPoolT, typename ThreadT>
  inline void GenericThreadPool<ThreadPoolT, ThreadT>::Stop()
  {
    std::call_once(
      DeinitOnce,
      [this] ()
      {
        IsStopping = true;
        Awaiting.notify_all();

        Threads.clear();
      });
  }

  template <typename ThreadPoolT, typename ThreadT>
  inline void GenericThreadPool<ThreadPoolT, ThreadT>::NotifyNewTask()
  {
    Awaiting.notify_one();
  }

  template <typename ThreadPoolT, typename ThreadT>
  inline void GenericThreadPool<ThreadPoolT, ThreadT>::Worker()
  {
    while (!IsStopping)
    {
      ThreadPoolInterface::Work work;

      {
        std::unique_lock<std::mutex> lock{AwaitingMutex};
        Awaiting.wait(
          lock,
          [this, &work] ()
          {
            work = std::move(PopNextTask());
            return IsStopping || work;
          });

        if (IsStopping)
          break;
      }

      if (work)
        work();
    }
  }

} }
