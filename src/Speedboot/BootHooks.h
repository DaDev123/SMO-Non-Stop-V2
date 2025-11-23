#pragma once

// borrowed from https://github.com/Amethyst-szs/shadow-clone-chasers

#include "Library/Nerve/NerveStateBase.h"

#include "Sequence/HakoniwaSequence.h"
#include "Sequence/WorldResourceLoader.h"

namespace speedboot {
    class HakoniwaSequenceSpeedboot : public al::NerveStateBase {
    public:
        HakoniwaSequenceSpeedboot(HakoniwaSequence* sequence);

        void exeInitThread();
        void exeLoadStage();
        void exeWipeToKill();

        bool isDoneLoading() const {
            return mSequence->mResourceLoader->isEndLoadWorldResource() && mSequence->mInitThread->isDone();
        }

    private:
        HakoniwaSequence* mSequence;
    };
} // namespace speedboot

void setupBootHooks();
