// Copyright 2024 TeiaCare
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <teiacare/sdk/task.hpp>
#include <teiacare/sdk/non_copyable.hpp>
#include <teiacare/sdk/non_moveable.hpp>

#include <atomic>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <future>
#include <condition_variable>

namespace tc::sdk
{
/*!
 * \class thread_pool
 * \brief Thread Pool that can run any callable object.
 * 
 * This class is general purpose thread pool that can launch task asyncronously.
 * It is possible to get an asyncronous result of a task execution.
 */
class thread_pool : private non_copyable, private non_moveable
{
public:
    /*!
     * \brief Constructor.
     * \param num_threads Number of threads that will be used in the underlying tc::sdk::thread_pool.
     * 
     * Creates a tc::sdk::thread_pool instance with the given number of threads.
     */
    explicit thread_pool();

    /*!
     * \brief Destructor.
     * 
     * Destructs this after all joinable threads are terminated.
     */
    ~thread_pool();

    /*!
     * \brief Starts thread pool.
     * 
     * Start num_threads workers threads. 
     * The actual number of threads is guaranteed to be in the range: [1, std::thread::hardware_concurrency()]
     */
    bool start(const unsigned int num_threads = std::thread::hardware_concurrency());

    /*!
     * \brief Stop all threads.
     * 
     * Stop thread pool and join all joinable threads.
     */
    bool stop();

    /*!
     * \brief Get the currently instanciated threads in the pool.
     * \return size_t number of threads
     * 
     * The number of threads is clamped in the range [1, std::thread::hardware_concurrency()]
     * so this value might be different from the parameter given to the tc::sdk::thread_pool::start(const unsigned int num_threads) method.
     */
    size_t threads_count() const;

    /*!
     * \brief Check if the pool is currently running.
     * \return true if running, otherwise false.
     * 
     */
    bool is_running() const;

    /*!
     * \brief Run a callable object asynchronously.
     * \tparam FunctionType Types of the callable object. 
     * \tparam f Callable object.
     * \return std::future task result
     * 
     * Enqueue a new task with the given callable object.
     * The enqueued task will run as soon as a thread is available.
     * Returns the result of the asynchronous computation.
     */
    template<typename FunctionType>
    auto run(FunctionType&& f)
    {
        using result_type = std::invoke_result_t<std::decay_t<FunctionType>>;
        std::packaged_task<result_type()> task(std::forward<FunctionType>(f));
        std::future<result_type> future = task.get_future();

        enqueue_task(tc::sdk::task(std::move(task)));
        return future;
    }

protected:
    void worker();
    void enqueue_task(tc::sdk::task&& task);

private:
    std::atomic_bool _is_running;
    std::mutex _is_running_mutex;
    std::vector<std::thread> _threads;
    std::queue<tc::sdk::task> _task_queue;
    std::condition_variable _task_cv;
    std::mutex _task_mutex;
};

}