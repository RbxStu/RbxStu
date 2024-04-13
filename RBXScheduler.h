//
// Created by Dottik on 13/4/2024.
//

#pragma once

#include <cstdint>
#include <chrono>
#include <condition_variable>
#include "StudioOffsets.h"

namespace RBX {
    class RunningAverage {
    public:
        double const lerp;
        double lastSampleValue;
        double averageValue;
        double averageVariance;
        unsigned char firstTime;
    };

    class Time {
    public:
        class Interval {
        public:
            double sec;
        };

        double sec;
    };

    class Timer {
    public:
        RBX::Time start;
    };


    class RunningAverageTimeInterval {
    public:
        class RBX::Timer timer;

        unsigned char firstTime;

        class RBX::RunningAverage average;

        unsigned char *ignoreTimeSinceLastSample;
    };


    class RunningAverageDutyCycle {
    public:
        class RBX::RunningAverage time;

        class RBX::RunningAverageTimeInterval interval;
    };

    class UnboundedCircularBuffer {
    public:
        uint64_t itemCapacity;
        uint64_t read;
        uint64_t itemCount;
        uint8_t *buffer;
    };


    class AsyncTaskQueue {
    public:
        struct QueueStats {
            uint64_t callCount;
            double total;
            double min;
            double max;
            double average;
        };

        std::mutex asyncTaskMutex;
        uint32_t numAsyncWorkersRequested;
        uint32_t numAsyncWorkersActive;
        std::condition_variable asyncWorkerWakeupCond;
        std::condition_variable asyncWorkerStoppedCond;

        class RBX::UnboundedCircularBuffer queue;

        struct RBX::AsyncTaskQueue::QueueStats executeTime;
    };

    struct CustomVector {
        void *first; // RBX::TaskScheduler::Job *
        void *__RefCounter____00;
        void *last; // RBX::TaskScheduler::Job *
        void *__RefCounter____01;
        void *end; // RBX::TaskScheduler::Job *
        void *__RefCounter____02;
    };


    struct TaskScheduler {
        uint32_t cycleCounter;
        bool cycleThrottling;

        class RunningAverageDutyCycle schedulerDutyCycle;

        class RBX::RunningAverage taskFrameTimeAverage;

        double previousFrameTime;

        class RBX::Timer stepTimer;

        double maxFrameTime;
        struct std::mutex globalMutex;
        std::condition_variable workersWakeupCond;

        class std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<__int64, std::ratio<1, 1000000000>>> nextCycleTimepoint;

        class RBX::Time::Interval cycleInterval;

        uint8_t backgroundMode;

        class RBX::Time::Interval cycleIntervalToRestore;

        char _170[0x10];
        char _180[0x18];
        struct RBX::CustomVector currentlyRunningJobs;
        char _1C8[0x18];
        char _1E0[0x20];
        char _200[8];
        void *taskQueue;// RBX::TaskQueue
        void *___ref__count__taskqueue0001;
        char _218[8];
        char _220[0x10];

        class RBX::AsyncTaskQueue asyncTaskQueue;

        char _368[8];
        char _370[8];
        char _378[8];
        char _380[8];
        char _388[8];
        char _390[8];
        char _398[8];
        char _3A0[8];
        char _3A8[0x10];
        char _3B8[8];
        char _3C0[8];
        char _3C8[8];

        static RBX::TaskScheduler *getSingleton() {
            return static_cast<RBX::TaskScheduler *>(RBX::Studio::Functions::rRBX__TaskScheduler__getSingleton());
        }
    };


}
