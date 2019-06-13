#include "stdafx.h"

#include "Disruptor/BlockingWaitStrategy.h"
#include "Disruptor/MultiProducerSequencer.h"


using namespace Disruptor;



TEST(ShouldOnlyAllowMessagesToBeAvailableIfSpecificallyPublished, MultiProducerSequencerTests)
{
    auto waitingStrategy = std::make_shared< BlockingWaitStrategy >();
    auto publisher = std::make_shared< MultiProducerSequencer< int > >(1024, waitingStrategy);

    publisher->publish(3);
    publisher->publish(5);

    EXPECT_EQ(publisher->isAvailable(0), false);
    EXPECT_EQ(publisher->isAvailable(1), false);
    EXPECT_EQ(publisher->isAvailable(2), false);
    EXPECT_EQ(publisher->isAvailable(3), true );
    EXPECT_EQ(publisher->isAvailable(4), false);
    EXPECT_EQ(publisher->isAvailable(5), true );
    EXPECT_EQ(publisher->isAvailable(6), false);
}

