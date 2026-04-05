// Copyright 2021 GHA Test Team

#include "TimedDoor.h"
#include <stdexcept>
#include <thread>
#include <chrono>

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d) : door(d) {}

void DoorTimerAdapter::Timeout() {
    if (door.isDoorOpened()) {
        door.throwState();
    }
}

TimedDoor::TimedDoor(int timeout) 
    : iTimeout(timeout), isOpened(false) {
    adapter = new DoorTimerAdapter(*this);
}

TimedDoor::~TimedDoor() {
    if (timerThread.joinable()) {
        timerThread.join();
    }
    delete adapter;
}

bool TimedDoor::isDoorOpened() {
    std::lock_guard<std::mutex> lock(doorMutex);
    return isOpened;
}

void TimedDoor::unlock() {
    std::lock_guard<std::mutex> lock(doorMutex);
    if (!isOpened) {
        if (timerThread.joinable()) {
            timerThread.join();
        }
        isOpened = true;
        Timer timer;
        timer.tregister(iTimeout, adapter);
    }
}

void TimedDoor::lock() {
    std::lock_guard<std::mutex> lock(doorMutex);
    if (isOpened) {
        if (timerThread.joinable()) {
            timerThread.join();
        }
        isOpened = false;
    }
}

int TimedDoor::getTimeOut() const {
    return iTimeout;
}

void TimedDoor::throwState() {
    throw std::runtime_error("Door is still open!");
}

void Timer::tregister(int seconds, TimerClient* client) {
    std::thread([seconds, client]() {
        std::this_thread::sleep_for(
            std::chrono::seconds(seconds));
        if (client) {
            client->Timeout();
        }
    }).detach();
}
