// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <thread>
#include <chrono>
#include "TimedDoor.h"

using ::testing::Return;
using ::testing::Test;

struct TimedDoorTest : public Test {
    TimedDoor* door;
    const int timeout = 1;

    void SetUp() override {
        door = new TimedDoor(timeout);
    }

    void TearDown() override {
        delete door;
    }
};

TEST_F(TimedDoorTest, IsInitiallyClosed) {
    ASSERT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
    door->unlock();
    ASSERT_TRUE(door->isDoorOpened());
    door->lock();
}

TEST_F(TimedDoorTest, LockClosesDoor) {
    door->unlock();
    door->lock();
    ASSERT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeoutReturnsCorrectValue) {
    ASSERT_EQ(door->getTimeOut(), timeout);
}

TEST_F(TimedDoorTest, ThrowStateThrowsRuntimeError) {
    ASSERT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, ThrowStateThrowsExceptionWithCorrectMessage) {
    try {
        door->throwState();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& err) {
        EXPECT_STREQ("Door is still open!", err.what());
    } catch (...) {
        FAIL() << "Expected std::runtime_error";
    }
}

TEST_F(TimedDoorTest, UnlockTwiceThenLock) {
    door->unlock();
    door->unlock();
    door->lock();
    std::this_thread::sleep_for(std::chrono::seconds(timeout + 1));
    ASSERT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, NoExceptionIfDoorClosedBeforeTimeout) {
    door->unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    door->lock();
    std::this_thread::sleep_for(std::chrono::seconds(timeout));
    SUCCEED();
}

TEST_F(TimedDoorTest, MultipleUnlockLockSequences) {
    for (int i = 0; i < 5; ++i) {
        door->unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        door->lock();
    }
    ASSERT_FALSE(door->isDoorOpened());
}

class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

TEST(TimerTest, TimerCallsTimeout) {
    MockTimerClient mockClient;
    Timer timer;

    EXPECT_CALL(mockClient, Timeout()).Times(1);

    timer.tregister(1, &mockClient);
    std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST(DoorTimerAdapterTest, TimeoutCallsThrowStateWhenDoorOpen) {
    TimedDoor door(1);
    DoorTimerAdapter adapter(door);

    door.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST(DoorTimerAdapterTest, TimeoutDoesNothingWhenDoorClosed) {
    TimedDoor door(1);
    DoorTimerAdapter adapter(door);

    door.lock();

    EXPECT_NO_THROW(adapter.Timeout());
}
