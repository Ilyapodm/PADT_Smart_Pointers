#pragma once

namespace test_utils {
    class LifetimeCounter {
    public:
        LifetimeCounter() {
            ++created;
            ++alive;
        }

        ~LifetimeCounter() {
            ++destroyed;
            --alive;
        }

        static void reset_counters() {
            created = 0;
            destroyed = 0;
            alive = 0;
        }

        // we need inline because of 
        static inline int created = 0;
        static inline int destroyed = 0;
        static inline int alive = 0;
    };
} 

