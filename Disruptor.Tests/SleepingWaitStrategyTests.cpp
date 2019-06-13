#include "stdafx.h"

#include "Disruptor/SleepingWaitStrategy.h"
#include "WaitStrategyTestUtil.h"


using namespace Disruptor;
using namespace Disruptor::Tests;

TEST(ShouldWaitForValue, SleepingWaitStrategyTests)
{
    assertWaitForWithDelayOf(50, std::make_shared< SleepingWaitStrategy >());
}

