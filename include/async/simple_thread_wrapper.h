#pragma once

#include <thread>

namespace Conjur { namespace Async {

  /**
   * @brief Implementation of a simple thread wrapper.
   */
  class SimpleThreadWrapper
  {
  public:

    SimpleThreadWrapper(const SimpleThreadWrapper&) = delete;
    SimpleThreadWrapper operator=(const SimpleThreadWrapper&) = delete;

    SimpleThreadWrapper(SimpleThreadWrapper&&) = default;
    SimpleThreadWrapper& operator=(SimpleThreadWrapper&&) = default;

    /**
     * @brief Constructs a new simple thread wrapper. 
     */
    template <typename... ArgsT>
    SimpleThreadWrapper(ArgsT&&... args);

    ~SimpleThreadWrapper();

  private:

    std::thread Thread;
  };

  template <typename... ArgsT>
  inline SimpleThreadWrapper::SimpleThreadWrapper(ArgsT&&... args)
    : Thread{std::forward<ArgsT>(args)...}
  {
  }

  inline SimpleThreadWrapper::~SimpleThreadWrapper()
  {
    if (Thread.joinable())
      Thread.join();
  }

} }
