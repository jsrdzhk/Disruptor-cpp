#include "stdafx.h"
#include "LatencyRecorder.h"

#include "Disruptor.TestTools/DurationHumanizer.h"
#include <numeric>


namespace Disruptor
{
namespace Tests
{

    struct DurationPrinter
    {
        explicit DurationPrinter(std::int64_t nanoseconds)
            : value(nanoseconds)
        {
        }

        std::int64_t value;
    };

    std::ostream& operator<<(std::ostream& stream, const DurationPrinter& printer)
    {
        auto humanDuration = Tests::DurationHumanizer::deduceHumanDuration(std::chrono::nanoseconds(printer.value));

        return stream << humanDuration.value << " " << humanDuration.shortUnitName;
    }


    LatencyRecorder::LatencyRecorder(std::int64_t sampleSize)
    {
        m_accumulator.resize(sampleSize);
    }

    void LatencyRecorder::record(std::int64_t value)
    {
        m_accumulator.push_back(value);
    }

    void LatencyRecorder::writeReport(std::ostream& stream)
    {
        const auto minMax = std::minmax(m_accumulator.begin(), m_accumulator.end());
        const auto mean = (std::int64_t)(std::accumulate(m_accumulator.begin(), m_accumulator.end(), 0 / (double)m_accumulator.size()));

        std::sort(m_accumulator.begin(), m_accumulator.end());
        const auto q50 = m_accumulator[std::size_t(m_accumulator.size() * 0.5)];
        const auto q90 = m_accumulator[std::size_t(m_accumulator.size() * 0.9)];
        const auto q93 = m_accumulator[std::size_t(m_accumulator.size() * 0.93)];
        const auto q95 = m_accumulator[std::size_t(m_accumulator.size() * 0.95)];
        const auto q98 = m_accumulator[std::size_t(m_accumulator.size() * 0.98)];
        const auto q99 = m_accumulator[std::size_t(m_accumulator.size() * 0.99)];
        const auto q999 = m_accumulator[std::size_t(m_accumulator.size() * 0.999)];
        const auto q9999 = m_accumulator[std::size_t(m_accumulator.size() * 0.9999)];

        stream
            << "min: " << DurationPrinter(*minMax.first)
            << ", mean: " << DurationPrinter(mean)
            << ", max: " << DurationPrinter(*minMax.second)
            << ", Q99.99: " << DurationPrinter(q9999)
            << ", Q99.9: " << DurationPrinter(q999)
            << ", Q99: " << DurationPrinter(q99)
            << ", Q98: " << DurationPrinter(q98)
            << ", Q95: " << DurationPrinter(q95)
            << ", Q93: " << DurationPrinter(q93)
            << ", Q90: " << DurationPrinter(q90)
            << ", Q50: " << DurationPrinter(q50)
            ;
    }

} // namespace Tests
} // namespace Disruptor
