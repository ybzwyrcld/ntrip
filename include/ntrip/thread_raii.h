// MIT License
//
// Copyright (c) 2022 Yuming Meng
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// @File    :  thread_raii.h
// @Version :  1.0
// @Time    :  2022/02/17 10:29:49
// @Author  :  Meng Yuming
// @Contact :  mengyuming@hotmail.com
// @Desc    :  None

#ifndef NTRIPLIB_THREAD_RAII_H_
#define NTRIPLIB_THREAD_RAII_H_

#include <thread>


namespace libntrip {

class Thread {
 public:
  Thread() = default;
  Thread(std::thread&& t) : thread_(std::move(t)) {}
  Thread(Thread const&) = delete;
  Thread(Thread&&) = delete;
  Thread& operator=(Thread const&) = delete;
  Thread& operator=(Thread&&) = delete;
  template<typename Callable, typename... Args>
  Thread(Callable&& callable, Args&&... args) : Thread(std::thread(
      std::forward<Callable>(callable), std::forward<Args>(args)...)) {}
  ~Thread() { join(); }

  template<typename Callable, typename... Args>
  Thread& reset(Callable&& callable, Args&&... args) {
    join();
    thread_ = std::move(std::thread(
        std::forward<Callable>(callable), std::forward<Args>(args)...));
    return *this;
  }
  Thread& reset(std::thread&& t) {
    join();
    thread_ = std::move(t);
    return *this;
  }
  void join(void) {
    if (thread_.joinable()) thread_.join();
  }
  void detach(void) {
    if (thread_.joinable()) thread_.detach();
  }

 private:
  std::thread thread_;
};

}  // namespace libntrip

#endif  // NTRIPLIB_THREAD_RAII_H_
