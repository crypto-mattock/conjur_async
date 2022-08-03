#pragma once

#include <functional>

namespace Conjur { namespace Async {

  /**
   * @brief Interface of a thread pool.
   */
  class ThreadPoolInterface
  {
  public:

    using Work = std::function<void(void)>;

    virtual ~ThreadPoolInterface() = default;

    /**
     * @brief Constructs the threads and starts the thread pool.
     */
    virtual void Start() = 0;

    /**
     * @brief Stops the thread pool and joins all threads.
     */
    virtual void Stop() = 0;
  };

} }
