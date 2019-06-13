#include "stdafx.h"

#include "LatencyTestSession.h"
#include "TestRepository.h"
#include "ThroughputTestSession.h"
#include <locale>

using namespace Disruptor::PerfTests;

void runAllTests(const TestRepository& testRepository);
void runOneTest(const TestRepository& testRepository, const std::string& testName);

int main(int, char**)
{
    auto& testRepository = TestRepository::instance();

    std::string testName;

    std::cout << "Test name (ALL by default):  " << testName << " ?" << std::endl;

    std::getline(std::cin, testName);

    testName.erase(testName.begin(), std::find_if(testName.begin(), testName.end(), [](int ch) { return !::isspace(ch); }));
    const auto allTests = std::string("All");

    auto runAll = testName.empty() || 
        testName.size() == allTests.size() &&
        std::equal(testName.begin(), 
            testName.end(), 
            allTests.begin(), 
            [](auto left, auto right) {return ::tolower(left) == ::tolower(right); });

    if (runAll)
        runAllTests(testRepository);
    else
        runOneTest(testRepository, testName);

    return 0;
}

void runAllTests(const TestRepository& testRepository)
{
    for (auto&& info : testRepository.allLatencyTests())
    {
        LatencyTestSession session(info);
        session.run();
    }

    for (auto&& info : testRepository.allThrougputTests())
    {
        ThroughputTestSession session(info);
        session.run();
    }
}

void runOneTest(const TestRepository& testRepository, const std::string& testName)
{
    LatencyTestInfo latencyTestInfo;
    if (testRepository.tryGetLatencyTest(testName, latencyTestInfo))
    {
        LatencyTestSession session(latencyTestInfo);
        session.run();
    }
    else
    {
        ThroughputTestInfo throughputTestInfo;
        if (testRepository.tryGetThroughputTest(testName, throughputTestInfo))
        {
            ThroughputTestSession session(throughputTestInfo);
            session.run();
        }
    }
}
