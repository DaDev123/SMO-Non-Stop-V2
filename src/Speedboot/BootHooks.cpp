#include "Speedboot/BootHooks.h"

#include <exlaunch.hpp>
#include <nn/fs.h>

#include "Library/Layout/LayoutInitInfo.h"
#include "Library/Nerve/Nerve.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveStateBase.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Play/Layout/WipeHolder.h"
#include "Library/Thread/AsyncFunctorThread.h"

#include "Sequence/ChangeStageInfo.h"
#include "Sequence/HakoniwaSequence.h"
#include "Sequence/WorldResourceLoader.h"
#include "System/GameDataFunction.h"
#include "System/WorldList.h"

#include "Speedboot/SpeedbootLoad.hpp"

// Define nerves for standard sequence
NERVE_IMPL(HakoniwaSequence, LoadStage)
NERVE_IMPL(HakoniwaSequence, LoadWorldResourceWithBoot)
NERVES_MAKE_NOSTRUCT(HakoniwaSequence, LoadStage, LoadWorldResourceWithBoot)

namespace speedboot {
    // Custom nerve that handles the speedboot flow
    class CustomBootNerve : public al::Nerve {
    public:
        void execute(al::NerveKeeper* keeper) const override {
            if (al::updateNerveState(keeper->mParent)) {
                al::setNerve(keeper->mParent, &LoadStage);
            }
        }
    };

    CustomBootNerve nrvSpeedboot;
    HakoniwaSequenceSpeedboot* speedbootState = nullptr;

    namespace {
        NERVE_IMPL(HakoniwaSequenceSpeedboot, InitThread)
        NERVE_IMPL(HakoniwaSequenceSpeedboot, LoadStage)
        NERVE_IMPL(HakoniwaSequenceSpeedboot, WipeToKill)
        NERVES_MAKE_NOSTRUCT(HakoniwaSequenceSpeedboot, InitThread, LoadStage, WipeToKill)
    }

    // Constructor
    HakoniwaSequenceSpeedboot::HakoniwaSequenceSpeedboot(HakoniwaSequence* sequence)
        : al::NerveStateBase("Speedboot"), mSequence(sequence) {
        initNerve(&LoadStage, 0);
    }

    // Initialize loading thread
    void HakoniwaSequenceSpeedboot::exeInitThread() {
        if (al::isFirstStep(this)) {
            mSequence->mInitThread->start();
        }

        if (mSequence->mInitThread->isDone()) {
            al::setNerve(this, &LoadStage);
        }
    }

    // Load stage resources
    void HakoniwaSequenceSpeedboot::exeLoadStage() {
        if (al::isFirstStep(this)) {
            mSequence->mInitThread->start();
            
            // Get stage name from game data
            const char* stageName = GameDataFunction::getNextStageName(mSequence->mGameDataHolderAccessor);
            if (!stageName) {
                stageName = GameDataFunction::getMainStageName(mSequence->mGameDataHolderAccessor, 0);
            }

            // Calculate scenario number
            s32 scenario = GameDataFunction::calcNextScenarioNo(mSequence->mGameDataHolderAccessor);
            if (scenario == -1) {
                scenario = 1;
            }

            // Request world resources
            s32 worldIndex = mSequence->mGameDataHolderAccessor->mWorldList->tryFindWorldIndexByStageName(stageName);
            if (worldIndex > -1) {
                mSequence->mResourceLoader->requestLoadWorldHomeStageResource(worldIndex, scenario);
            }
        }

        if (isDoneLoading()) {
            al::setNerve(this, &WipeToKill);
        }
    }

    // Fade to black and complete speedboot
    void HakoniwaSequenceSpeedboot::exeWipeToKill() {
        if (al::isFirstStep(this)) {
            mSequence->mWipeHolder->startClose("FadeBlack", -1);
        }

        if (mSequence->mWipeHolder->isCloseEnd()) {
            kill();
        }
    }

    // Check if loading is complete
    bool HakoniwaSequenceSpeedboot::isDoneLoading() const {
        return mSequence->mResourceLoader->isEndLoadWorldResource() 
            && mSequence->mInitThread->isDone();
    }

    // Prepare layout initialization with proper stage name handling
    extern "C" void _ZN10BootLayoutC1ERKN2al14LayoutInitInfoE(BootLayout* layout,
                                                              const al::LayoutInitInfo& layoutInitInfo);

    void prepareLayoutInitInfo(BootLayout* layout, const al::LayoutInitInfo& layoutInitInfo) {
        register HakoniwaSequence* sequence asm("x19");

        // Create speedboot load screen
        new SpeedbootLoad(sequence->mResourceLoader, layoutInitInfo, sequence);
        
        // Initialize boot layout
        _ZN10BootLayoutC1ERKN2al14LayoutInitInfoE(layout, layoutInitInfo);
    }

    // Setup custom nerve for speedboot sequence
    void hakoniwaSetNerveSetup(al::IUseNerve* useNerve, al::Nerve* nerve) {
        al::setNerve(useNerve, &nrvSpeedboot);
        
        auto* sequence = static_cast<HakoniwaSequence*>(useNerve);
        speedbootState = new HakoniwaSequenceSpeedboot(sequence);
        
        al::initNerveState(useNerve, speedbootState, &nrvSpeedboot, "Speedboot");
    }
}

// Install code patches for speedboot functionality
void setupBootHooks() {
    // Patch nerve setup function
    exl::patch::CodePatcher(0x50ef28).BranchLinkInst((void*)&speedboot::hakoniwaSetNerveSetup);
    
    // Increase nerve count to accommodate custom nerves
    exl::patch::CodePatcher(0x50eb88).WriteInst(exl::armv8::inst::Movz(exl::armv8::reg::W2, 0x1f));
    
    // Patch layout initialization
    exl::patch::CodePatcher(0x50eb64).BranchLinkInst((void*)&speedboot::prepareLayoutInitInfo);
}