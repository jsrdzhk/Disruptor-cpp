#include "stdafx.h"

#include "Disruptor/ArgumentException.h"
#include "Disruptor/FatalExceptionHandler.h"
#include "StubEvent.h"


using namespace Disruptor;


TEST(FatalExceptionHandlerTests, ShouldHandleFatalException)
{
    auto causeException = ArgumentException("FatalExceptionHandlerTests.ShouldHandleFatalException");
    auto evt = Tests::StubEvent(0);

    auto exceptionHandler = std::make_shared< FatalExceptionHandler< Tests::StubEvent> >();

    try
    {
        exceptionHandler->handleEventException(causeException, 0L, evt);
    }
    catch (FatalException& ex)
    {
        EXPECT_STREQ(causeException.what(), ex.innerException().what());
    }
}
