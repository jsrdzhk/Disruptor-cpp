#include "stdafx.h"

#include "Disruptor/BusySpinWaitStrategy.h"
#include "WaitStrategyTestUtil.h"


using namespace Disruptor;
using namespace Disruptor::Tests;

TEST(BusySpinWaitStrategyTests, ShouldWaitForValue)
{
    assertWaitForWithDelayOf(50, std::make_shared< BusySpinWaitStrategy >());
}
