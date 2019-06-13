#include "stdafx.h"
#include "PerfTestUtil.h"

#include <chrono>
#include <ctime>

#include "Disruptor/IEventProcessor.h"
#include "Disruptor/ISequence.h"


namespace Disruptor
{
namespace PerfTests
{
namespace PerfTestUtil
{

    std::int64_t accumulatedAddition(std::int64_t iterations)
    {
        std::int64_t temp = 0L;
        for (std::int64_t i = 0L; i < iterations; i++)
        {
            temp += i;
        }

        return temp;
    }

    void failIf(std::int64_t a, std::int64_t b, const std::string& message)
    {
        if (a == b)
        {
            throw std::runtime_error(message.empty() ? "Test failed " + std::to_string(a) + " == " + std::to_string(b) : message);
        }
    }

    void failIfNot(std::int64_t a, std::int64_t b, const std::string& message)
    {
        if (a != b)
        {
            throw std::runtime_error(message.empty() ? "Test failed " + std::to_string(a) + " != " + std::to_string(b) : message);
        }
    }

    void waitForEventProcessorSequence(std::int64_t expectedCount, const std::shared_ptr< IEventProcessor >& batchEventProcessor)
    {
        while (batchEventProcessor->sequence()->value() != expectedCount)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    std::string utcDateToString()
    {
        const auto t(std::chrono::system_clock::now());
        auto time = std::chrono::system_clock::to_time_t(t);
        return std::ctime(&time);
    }

} // namespace PerfTestUtil
} // namespace PerfTests
} // namespace Disruptor
