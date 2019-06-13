#pragma once
#include <condition_variable>

namespace Disruptor
{
    namespace Tests
    {

        // until there is a real support of std::barrier
        class Barrier
        {
        public:
            explicit Barrier(std::size_t count)
                : m_currentCount(count)
            { }

            void wait()
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                if (--m_currentCount == 0)
                    m_conditionVariable.notify_all();
                else
                    m_conditionVariable.wait(lock, [this] { return m_currentCount == 0; });
            }

        private:
            std::mutex m_mutex;
            std::condition_variable m_conditionVariable;
            std::size_t m_currentCount;
        };

    }
}
