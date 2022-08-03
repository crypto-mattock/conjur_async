#pragma once

#include <async/thread_pool_interface.h>

namespace Conjur { namespace Async {

  /**
   * @brief Interface of a simple thread pool.
   */
  class SimpleThreadPoolInterface
    : public ThreadPoolInterface
  {
  public:

    virtual ~SimpleThreadPoolInterface() = default;

    /**
     * @brief Posts a task to the thread pool.
     * @param work task
     */
    virtual void Post(ThreadPoolInterface::Work&& work) = 0;
  };

} }
