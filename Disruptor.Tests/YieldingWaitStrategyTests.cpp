#include "stdafx.h"

#include "Disruptor/YieldingWaitStrategy.h"
#include "WaitStrategyTestUtil.h"


using namespace Disruptor;
using namespace Disruptor::Tests;

TEST(ShouldWaitForValue, YieldingWaitStrategyTests)
{
    assertWaitForWithDelayOf(50, std::make_shared< YieldingWaitStrategy >());
}
