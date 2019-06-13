#include "stdafx.h"
#include "TestRepository.h"

// Raw
#include "OneToOneRawBatchThroughputTest.h"
#include "OneToOneRawThroughputTest.h"

// Sequenced
#include "OneToOneSequencedBatchThroughputTest.h"
#include "OneToOneSequencedLongArrayThroughputTest.h"
#include "OneToOneSequencedPollerThroughputTest.h"
#include "OneToOneSequencedThroughputTest.h"
#include "OneToThreeDiamondSequencedThroughputTest.h"
#include "OneToThreePipelineSequencedThroughputTest.h"
#include "OneToThreeSequencedThroughputTest.h"
#include "PingPongSequencedLatencyTest.h"
#include "ThreeToOneSequencedBatchThroughputTest.h"
#include "ThreeToOneSequencedThroughputTest.h"
#include "ThreeToThreeSequencedThroughputTest.h"

// Translator
#include "OneToOneTranslatorThroughputTest.h"

// WorkHandler
#include "OneToThreeReleasingWorkerPoolThroughputTest.h"
#include "OneToThreeWorkerPoolThroughputTest.h"
#include "TwoToTwoWorkProcessorThroughputTest.h"


namespace Disruptor
{
namespace PerfTests
{

    TestRepository::TestRepository()
    {
        // Raw
        registerTest< OneToOneRawBatchThroughputTest >();
        registerTest< OneToOneRawThroughputTest >();

        // Sequenced
        registerTest< OneToOneSequencedBatchThroughputTest >();
        registerTest< OneToOneSequencedLongArrayThroughputTest >();
        registerTest< OneToOneSequencedPollerThroughputTest >();
        registerTest< OneToOneSequencedThroughputTest >();
        registerTest< OneToThreeDiamondSequencedThroughputTest >();
        registerTest< OneToThreePipelineSequencedThroughputTest >();
        registerTest< OneToThreeSequencedThroughputTest >();
        registerTest< PingPongSequencedLatencyTest >();
        registerTest< ThreeToOneSequencedBatchThroughputTest >();
        registerTest< ThreeToOneSequencedThroughputTest >();
        registerTest< ThreeToThreeSequencedThroughputTest >();

        // Translator
        registerTest< OneToOneTranslatorThroughputTest >();

        // WorkHandler
        registerTest< OneToThreeReleasingWorkerPoolThroughputTest >();
        registerTest< OneToThreeWorkerPoolThroughputTest >();
        registerTest< TwoToTwoWorkProcessorThroughputTest >();
    }

    void TestRepository::registerTest(const TypeInfo& typeInfo, const std::function<std::shared_ptr< IThroughputTest >()>& testFactory)
    {
        ThroughputTestInfo info{ typeInfo.name(), testFactory };

        auto fullyQualifiedName = typeInfo.fullyQualifiedName();
        std::transform(fullyQualifiedName.begin(), fullyQualifiedName.end(), fullyQualifiedName.begin(), ::tolower);

        auto typeInfoName = typeInfo.name();
        std::transform(typeInfoName.begin(), typeInfoName.end(), typeInfoName.begin(), ::tolower);

        m_throughputTestInfosByName.insert(std::make_pair(fullyQualifiedName, info));
        m_throughputTestInfosByName.insert(std::make_pair(typeInfoName, info));
    }

    void TestRepository::registerTest(const TypeInfo& typeInfo, const std::function<std::shared_ptr< ILatencyTest >()>& testFactory)
    {
        LatencyTestInfo info{ typeInfo.name(), testFactory };

        auto fullyQualifiedName = typeInfo.fullyQualifiedName();
        std::transform(fullyQualifiedName.begin(), fullyQualifiedName.end(), fullyQualifiedName.begin(), ::tolower);

        auto typeInfoName = typeInfo.name();
        std::transform(typeInfoName.begin(), typeInfoName.end(), typeInfoName.begin(), ::tolower);

        m_latencyTestInfosByName.insert(std::make_pair(fullyQualifiedName, info));
        m_latencyTestInfosByName.insert(std::make_pair(typeInfoName, info));
    }

    const TestRepository& TestRepository::instance()
    {
        static TestRepository instance;
        return instance;
    }

    std::vector< ThroughputTestInfo > TestRepository::allThrougputTests() const
    {
        std::vector< ThroughputTestInfo > result;
        std::set< std::string > testNames;

        for (auto&& x : m_throughputTestInfosByName)
        {
            if (testNames.count(x.second.name) > 0)
                continue;

            testNames.insert(x.second.name);
            result.push_back(x.second);
        }
        return result;
    }

    bool TestRepository::tryGetThroughputTest(std::string testName, ThroughputTestInfo& testInfo) const
    {
        std::transform(testName.begin(), testName.end(), testName.begin(), ::tolower);

        const auto it = m_throughputTestInfosByName.find(testName);
        if (it == m_throughputTestInfosByName.end())
            return false;

        testInfo = it->second;
        return true;
    }

    std::vector< LatencyTestInfo > TestRepository::allLatencyTests() const
    {
        std::vector< LatencyTestInfo > result;
        std::set< std::string > testNames;

        for (auto&& x : m_latencyTestInfosByName)
        {
            if (testNames.count(x.second.name) > 0)
                continue;

            testNames.insert(x.second.name);
            result.push_back(x.second);
        }
        return result;
    }

    bool TestRepository::tryGetLatencyTest(std::string testName, LatencyTestInfo& testInfo) const
    {
        std::transform(testName.begin(), testName.end(), testName.begin(), ::tolower);

        auto it = m_latencyTestInfosByName.find(testName);
        if (it == m_latencyTestInfosByName.end())
            return false;

        testInfo = it->second;
        return true;
    }

} // namespace PerfTests
} // namespace Disruptor
