#pragma once

#include <cstdint>

#include "Disruptor/Pragmas.h"

namespace Disruptor
{
namespace Tests
{

    class LatencyRecorder
    {
        /*using Accumulator = boost::accumulators::accumulator_set
        <
            double,
            boost::accumulators::stats
            <
                boost::accumulators::tag::mean,
                boost::accumulators::tag::max,
                boost::accumulators::tag::min,
                boost::accumulators::tag::variance,
                boost::accumulators::tag::tail_quantile< boost::accumulators::right >
            >
        >;
*/
    public:
        explicit LatencyRecorder(std::int64_t sampleSize);
        
        void record(std::int64_t value);

        void writeReport(std::ostream& stream);

    private:
        std::vector<std::int64_t> m_accumulator;
    };

} // namespace Tests
} // namespace Disruptor
