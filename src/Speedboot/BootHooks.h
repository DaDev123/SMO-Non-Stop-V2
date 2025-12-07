#pragma once

#include "Library/Nerve/NerveStateBase.h"

class HakoniwaSequence;

namespace speedboot {
    /**
     * Custom nerve state for handling speedboot loading sequence
     * Manages initialization, resource loading, and transitions
     */
    class HakoniwaSequenceSpeedboot : public al::NerveStateBase {
    public:
        explicit HakoniwaSequenceSpeedboot(HakoniwaSequence* sequence);

        // Nerve execution functions
        void exeInitThread();
        void exeLoadStage();
        void exeWipeToKill();

        /**
         * Check if both world resources and initialization thread are complete
         */
        bool isDoneLoading() const;

    private:
        HakoniwaSequence* mSequence;
    };
}

/**
 * Initialize code patches for speedboot functionality
 * Must be called during mod initialization
 */
void setupBootHooks();