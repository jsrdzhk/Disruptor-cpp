#include "stdafx.h"

#include "Disruptor.TestTools/ManualResetEvent.h"

#include "SequencerFixture.h"
#include "WaitStrategyMock.h"


using namespace Disruptor;
using namespace Disruptor::Tests;


//typedef boost::mpl::vector< MultiProducerSequencer< int >, SingleProducerSequencer< int > > Sequencers;
typedef SequencerTestFixture<MultiProducerSequencer<int>> MultiProducerSequencerFixture;
typedef SequencerTestFixture<SingleProducerSequencer<int>> SingleProducerSequencerFixture;

TEST_F(MultiProducerSequencerFixture, ShouldStartWithInitialValue)
{
    EXPECT_EQ(this->m_sequencer->next(), 0);
}

TEST_F(MultiProducerSequencerFixture, ShouldBatchClaim)
{
    EXPECT_EQ(this->m_sequencer->next(4), 3);
}

TEST_F(MultiProducerSequencerFixture, ShouldIndicateHasAvailableCapacity)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(1), true);
    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(this->m_bufferSize), true);
    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(this->m_bufferSize + 1), false);

    this->m_sequencer->publish(this->m_sequencer->next());

    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(this->m_bufferSize - 1), true);
    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(this->m_bufferSize), false);
}

TEST_F(MultiProducerSequencerFixture, ShouldIndicateNoAvailableCapacity)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    auto sequence = this->m_sequencer->next(this->m_bufferSize);
    this->m_sequencer->publish(sequence - (this->m_bufferSize - 1), sequence);

    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(1), false);
}

TEST_F(MultiProducerSequencerFixture, ShouldHoldUpPublisherWhenBufferIsFull)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    auto sequence = this->m_sequencer->next(this->m_bufferSize);
    this->m_sequencer->publish(sequence - (this->m_bufferSize - 1), sequence);

    auto waitingSignal = std::make_shared< ManualResetEvent >(false);
    auto doneSignal = std::make_shared< ManualResetEvent >(false);

    auto expectedFullSequence = Sequence::InitialCursorValue + this->m_sequencer->bufferSize();
    EXPECT_EQ(this->m_sequencer->cursor(), expectedFullSequence);

    std::thread([=]
    {
        waitingSignal->set();

        auto next = this->m_sequencer->next();
        this->m_sequencer->publish(next);

        doneSignal->set();
    }).detach();

    waitingSignal->wait(std::chrono::milliseconds(500));
    EXPECT_EQ(this->m_sequencer->cursor(), expectedFullSequence);

    this->m_gatingSequence->setValue(Sequence::InitialCursorValue + 1L);

    doneSignal->wait(std::chrono::milliseconds(500));
    EXPECT_EQ(this->m_sequencer->cursor(), expectedFullSequence + 1L);
}

TEST_F(MultiProducerSequencerFixture, ShouldThrowInsufficientCapacityExceptionWhenSequencerIsFull)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    for (auto i = 0; i < this->m_bufferSize; ++i)
    {
        this->m_sequencer->next();
    }

    EXPECT_THROW(this->m_sequencer->tryNext(), InsufficientCapacityException);
}

TEST_F(MultiProducerSequencerFixture, ShouldCalculateRemainingCapacity)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    EXPECT_EQ(this->m_sequencer->getRemainingCapacity(), this->m_bufferSize);
    
    for (auto i = 1; i < this->m_bufferSize; ++i)
    {
        this->m_sequencer->next();
        EXPECT_EQ(this->m_sequencer->getRemainingCapacity(), this->m_bufferSize - i);
    }
}

TEST_F(MultiProducerSequencerFixture, ShoundNotBeAvailableUntilPublished)
{
    auto next = this->m_sequencer->next(6);

    for (auto i = 0; i <= 5; i++)
    {
        EXPECT_EQ(this->m_sequencer->isAvailable(i), false);
    }

    this->m_sequencer->publish(next - (6 - 1), next);
    
    for (auto i = 0; i <= 5; i++)
    {
        EXPECT_EQ(this->m_sequencer->isAvailable(i), true);
    }

    EXPECT_EQ(this->m_sequencer->isAvailable(6), false);
}

TEST_F(MultiProducerSequencerFixture, ShouldNotifyWaitStrategyOnPublish)
{
    auto waitStrategyMock = std::make_shared< testing::NiceMock< WaitStrategyMock > >();
    auto sequencer = std::make_shared< TSequencer >(this->m_bufferSize, waitStrategyMock);

    EXPECT_CALL(*waitStrategyMock, signalAllWhenBlocking()).Times(1);

    sequencer->publish(sequencer->next());
}

TEST_F(MultiProducerSequencerFixture, ShouldNotifyWaitStrategyOnPublishBatch)
{
    auto waitStrategyMock = std::make_shared< testing::NiceMock< WaitStrategyMock > >();
    auto sequencer = std::make_shared< TSequencer >(this->m_bufferSize, waitStrategyMock);

    EXPECT_CALL(*waitStrategyMock, signalAllWhenBlocking()).Times(1);

    auto next = sequencer->next(4);
    sequencer->publish(next - (4 - 1), next);
}

TEST_F(MultiProducerSequencerFixture, ShouldWaitOnPublication)
{
    auto barrier = this->m_sequencer->newBarrier({});

    auto next = this->m_sequencer->next(10);
    auto lo = next - (10 - 1);
    auto mid = next - 5;

    for (auto l = lo; l < mid; ++l)
    {
       this->m_sequencer->publish(l);
    }

    EXPECT_EQ(barrier->waitFor(-1), mid - 1);

    for (auto l = mid; l <= next; ++l)
    {
        this->m_sequencer->publish(l);
    }

    EXPECT_EQ(barrier->waitFor(-1), next);
}

TEST_F(MultiProducerSequencerFixture, ShouldTryNext)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    for (auto i = 0; i < this->m_bufferSize; i++)
    {
        EXPECT_NO_THROW(this->m_sequencer->publish(this->m_sequencer->tryNext()));
    }
 
    EXPECT_THROW(this->m_sequencer->tryNext(), InsufficientCapacityException);
}

TEST_F(MultiProducerSequencerFixture, ShouldClaimSpecificSequence)
{
    std::int64_t sequence = 14;

    this->m_sequencer->claim(sequence);
    this->m_sequencer->publish(sequence);

    EXPECT_EQ(this->m_sequencer->next(), sequence + 1);
}

TEST_F(MultiProducerSequencerFixture, ShouldNotAllowBulkNextLessThanZero)
{
    EXPECT_THROW(this->m_sequencer->next(-1), ArgumentException);
}

TEST_F(MultiProducerSequencerFixture, ShouldNotAllowBulkNextOfZero)
{
    EXPECT_THROW(this->m_sequencer->next(0), ArgumentException);
}

TEST_F(MultiProducerSequencerFixture, ShouldNotAllowBulkTryNextLessThanZero)
{
    EXPECT_THROW(this->m_sequencer->tryNext(-1), ArgumentException);
}

TEST_F(MultiProducerSequencerFixture, ShouldNotAllowBulkTryNextOfZero)
{
    EXPECT_THROW(this->m_sequencer->tryNext(0), ArgumentException);
}


TEST_F(SingleProducerSequencerFixture, ShouldStartWithInitialValue)
{
    EXPECT_EQ(this->m_sequencer->next(), 0);
}

TEST_F(SingleProducerSequencerFixture, ShouldBatchClaim)
{
    EXPECT_EQ(this->m_sequencer->next(4), 3);
}

TEST_F(SingleProducerSequencerFixture, ShouldIndicateHasAvailableCapacity)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(1), true);
    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(this->m_bufferSize), true);
    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(this->m_bufferSize + 1), false);

    this->m_sequencer->publish(this->m_sequencer->next());

    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(this->m_bufferSize - 1), true);
    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(this->m_bufferSize), false);
}

TEST_F(SingleProducerSequencerFixture, ShouldIndicateNoAvailableCapacity)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    auto sequence = this->m_sequencer->next(this->m_bufferSize);
    this->m_sequencer->publish(sequence - (this->m_bufferSize - 1), sequence);

    EXPECT_EQ(this->m_sequencer->hasAvailableCapacity(1), false);
}

TEST_F(SingleProducerSequencerFixture, ShouldHoldUpPublisherWhenBufferIsFull)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    auto sequence = this->m_sequencer->next(this->m_bufferSize);
    this->m_sequencer->publish(sequence - (this->m_bufferSize - 1), sequence);

    auto waitingSignal = std::make_shared< ManualResetEvent >(false);
    auto doneSignal = std::make_shared< ManualResetEvent >(false);

    auto expectedFullSequence = Sequence::InitialCursorValue + this->m_sequencer->bufferSize();
    EXPECT_EQ(this->m_sequencer->cursor(), expectedFullSequence);

    std::thread([=]
        {
            waitingSignal->set();

            auto next = this->m_sequencer->next();
            this->m_sequencer->publish(next);

            doneSignal->set();
        }).detach();

        waitingSignal->wait(std::chrono::milliseconds(500));
        EXPECT_EQ(this->m_sequencer->cursor(), expectedFullSequence);

        this->m_gatingSequence->setValue(Sequence::InitialCursorValue + 1L);

        doneSignal->wait(std::chrono::milliseconds(500));
        EXPECT_EQ(this->m_sequencer->cursor(), expectedFullSequence + 1L);
}

TEST_F(SingleProducerSequencerFixture, ShouldThrowInsufficientCapacityExceptionWhenSequencerIsFull)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    for (auto i = 0; i < this->m_bufferSize; ++i)
    {
        this->m_sequencer->next();
    }

    EXPECT_THROW(this->m_sequencer->tryNext(), InsufficientCapacityException);
}

TEST_F(SingleProducerSequencerFixture, ShouldCalculateRemainingCapacity)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    EXPECT_EQ(this->m_sequencer->getRemainingCapacity(), this->m_bufferSize);

    for (auto i = 1; i < this->m_bufferSize; ++i)
    {
        this->m_sequencer->next();
        EXPECT_EQ(this->m_sequencer->getRemainingCapacity(), this->m_bufferSize - i);
    }
}

TEST_F(SingleProducerSequencerFixture, ShoundNotBeAvailableUntilPublished)
{
    auto next = this->m_sequencer->next(6);

    for (auto i = 0; i <= 5; i++)
    {
        EXPECT_EQ(this->m_sequencer->isAvailable(i), false);
    }

    this->m_sequencer->publish(next - (6 - 1), next);

    for (auto i = 0; i <= 5; i++)
    {
        EXPECT_EQ(this->m_sequencer->isAvailable(i), true);
    }

    EXPECT_EQ(this->m_sequencer->isAvailable(6), false);
}

TEST_F(SingleProducerSequencerFixture, ShouldNotifyWaitStrategyOnPublish)
{
    auto waitStrategyMock = std::make_shared< testing::NiceMock< WaitStrategyMock > >();
    auto sequencer = std::make_shared< TSequencer >(this->m_bufferSize, waitStrategyMock);

    EXPECT_CALL(*waitStrategyMock, signalAllWhenBlocking()).Times(1);

    sequencer->publish(sequencer->next());
}

TEST_F(SingleProducerSequencerFixture, ShouldNotifyWaitStrategyOnPublishBatch)
{
    auto waitStrategyMock = std::make_shared< testing::NiceMock< WaitStrategyMock > >();
    auto sequencer = std::make_shared< TSequencer >(this->m_bufferSize, waitStrategyMock);

    EXPECT_CALL(*waitStrategyMock, signalAllWhenBlocking()).Times(1);

    auto next = sequencer->next(4);
    sequencer->publish(next - (4 - 1), next);
}

TEST_F(SingleProducerSequencerFixture, ShouldWaitOnPublication)
{
    auto barrier = this->m_sequencer->newBarrier({});

    auto next = this->m_sequencer->next(10);
    auto lo = next - (10 - 1);
    auto mid = next - 5;

    for (auto l = lo; l < mid; ++l)
    {
        this->m_sequencer->publish(l);
    }

    EXPECT_EQ(barrier->waitFor(-1), mid - 1);

    for (auto l = mid; l <= next; ++l)
    {
        this->m_sequencer->publish(l);
    }

    EXPECT_EQ(barrier->waitFor(-1), next);
}

TEST_F(SingleProducerSequencerFixture, ShouldTryNext)
{
    this->m_sequencer->addGatingSequences({ this->m_gatingSequence });

    for (auto i = 0; i < this->m_bufferSize; i++)
    {
        EXPECT_NO_THROW(this->m_sequencer->publish(this->m_sequencer->tryNext()));
    }

    EXPECT_THROW(this->m_sequencer->tryNext(), InsufficientCapacityException);
}

TEST_F(SingleProducerSequencerFixture, ShouldClaimSpecificSequence)
{
    std::int64_t sequence = 14;

    this->m_sequencer->claim(sequence);
    this->m_sequencer->publish(sequence);

    EXPECT_EQ(this->m_sequencer->next(), sequence + 1);
}

TEST_F(SingleProducerSequencerFixture, ShouldNotAllowBulkNextLessThanZero)
{
    EXPECT_THROW(this->m_sequencer->next(-1), ArgumentException);
}

TEST_F(SingleProducerSequencerFixture, ShouldNotAllowBulkNextOfZero)
{
    EXPECT_THROW(this->m_sequencer->next(0), ArgumentException);
}

TEST_F(SingleProducerSequencerFixture, ShouldNotAllowBulkTryNextLessThanZero)
{
    EXPECT_THROW(this->m_sequencer->tryNext(-1), ArgumentException);
}

TEST_F(SingleProducerSequencerFixture, ShouldNotAllowBulkTryNextOfZero)
{
    EXPECT_THROW(this->m_sequencer->tryNext(0), ArgumentException);
}