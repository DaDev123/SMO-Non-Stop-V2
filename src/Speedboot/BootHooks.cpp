#include "Speedboot/BootHooks.h"

#include <exlaunch.hpp>

#include <nn/fs.h>

#include "Library/Layout/LayoutInitInfo.h"
#include "Library/Nerve/Nerve.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveStateBase.h"
#include "Library/Nerve/NerveUtil.h"
#include "Library/Play/Layout/WipeHolder.h"

#include "Sequence/ChangeStageInfo.h"
#include "Sequence/HakoniwaSequence.h"
#include "System/GameDataFunction.h"
#include "System/WorldList.h"

#include "Speedboot/SpeedbootLoad.hpp"

NERVE_IMPL(HakoniwaSequence, LoadStage)
NERVE_IMPL(HakoniwaSequence, LoadWorldResourceWithBoot)

NERVES_MAKE_NOSTRUCT(HakoniwaSequence, LoadStage, LoadWorldResourceWithBoot)

namespace speedboot {
    class CustomBootNerve : public al::Nerve {
    public:
        void execute(al::NerveKeeper* keeper) const override {
            // auto* sequence = static_cast<HakoniwaSequence*>(keeper->mParent);
            if (al::updateNerveState(keeper->mParent)) {
                al::setNerve(keeper->mParent, &LoadStage);
            }
        }
    };

    CustomBootNerve nrvSpeedboot;
    al::LayoutInitInfo copiedInitInfo;
    const char* currentStageName = nullptr; // Store stage name globally

    namespace {
        NERVE_IMPL(HakoniwaSequenceSpeedboot, InitThread)
        NERVE_IMPL(HakoniwaSequenceSpeedboot, LoadStage)
        NERVE_IMPL(HakoniwaSequenceSpeedboot, WipeToKill)

        NERVES_MAKE_NOSTRUCT(HakoniwaSequenceSpeedboot, InitThread, LoadStage, WipeToKill)
    } // namespace

    HakoniwaSequenceSpeedboot::HakoniwaSequenceSpeedboot(HakoniwaSequence* sequence)
        : al::NerveStateBase("Speedboot"), mSequence(sequence) {
        initNerve(&LoadStage, 0);
    }

    void HakoniwaSequenceSpeedboot::exeInitThread() {
        if (al::isFirstStep(this)) {
            mSequence->mInitThread->start();
        }

        if (mSequence->mInitThread->isDone()) {
            al::setNerve(this, &LoadStage);
        }
    }
    void HakoniwaSequenceSpeedboot::exeLoadStage() {
        if (al::isFirstStep(this)) {
            mSequence->mInitThread->start();
            const char* name = GameDataFunction::getNextStageName(mSequence->mGameDataHolderAccessor);
            if (name == nullptr)
                name = GameDataFunction::getMainStageName(mSequence->mGameDataHolderAccessor, 0);
            
            // Store the stage name for SpeedbootLoad to use
            currentStageName = name;
            
            s32 scenario = GameDataFunction::calcNextScenarioNo(mSequence->mGameDataHolderAccessor);
            if (scenario == -1)
                scenario = 1;
            s32 world = mSequence->mGameDataHolderAccessor->mWorldList->tryFindWorldIndexByStageName(name);
            if (world > -1) {
                // Logger::log("Requesting %d-%d\n", world, scenario);
                mSequence->mResourceLoader->requestLoadWorldHomeStageResource(world, scenario);
            }
        }
        // Logger::log("Percentage %.02f, %s\n", mSequence->mResourceLoader->calcLoadPercent(),
        //             mSequence->mResourceLoader->isEndLoadWorldResource() ? "true" : "false");

        if (isDoneLoading()) {
            al::setNerve(this, &WipeToKill);
        }
    }

    void HakoniwaSequenceSpeedboot::exeWipeToKill() {
        if (al::isFirstStep(this))
            mSequence->mWipeHolder->startClose("FadeBlack", -1);

        if (mSequence->mWipeHolder->isCloseEnd()) {
            kill();
        }
    }

    HakoniwaSequenceSpeedboot* deezNutsState;

    extern "C" void _ZN10BootLayoutC1ERKN2al14LayoutInitInfoE(BootLayout* layout,
                                                              const al::LayoutInitInfo& layoutInitInfo);

    void prepareLayoutInitInfo(BootLayout* layout, const al::LayoutInitInfo& layoutInitInfo) {
        register HakoniwaSequence* sequence asm("x19");
        
        // Get the stage name from the sequence's mStageName member
        const char* name = nullptr;
        
        // First try the stage name stored in the sequence itself
        if (!sequence->mStageName.isEmpty()) {
            name = sequence->mStageName.cstr();
        }
        
        // If that's empty, try getNextStageName
        if (name == nullptr || name[0] == '\0') {
            name = GameDataFunction::getNextStageName(sequence->mGameDataHolderAccessor);
        }
        
        // Last resort: get current stage from save data
        if (name == nullptr || name[0] == '\0') {
            name = GameDataFunction::getCurrentStageName(sequence->mGameDataHolderAccessor);
        }
        
        currentStageName = name;
        
        new SpeedbootLoad(sequence->mResourceLoader, layoutInitInfo, currentStageName);
        _ZN10BootLayoutC1ERKN2al14LayoutInitInfoE(layout, layoutInitInfo);
    }

    void hakoniwaSetNerveSetup(al::IUseNerve* useNerve, al::Nerve* nerve) {
        al::setNerve(useNerve, &nrvSpeedboot);
        auto* sequence = static_cast<HakoniwaSequence*>(useNerve);
        deezNutsState = new HakoniwaSequenceSpeedboot(sequence);
        al::initNerveState(useNerve, deezNutsState, &nrvSpeedboot, "Speedboot");
    }
} // namespace speedboot

void setupBootHooks() {
    exl::patch::CodePatcher(0x50ef28).BranchLinkInst((void*)&speedboot::hakoniwaSetNerveSetup);
    exl::patch::CodePatcher(0x50eb88).WriteInst(
        exl::armv8::inst::Movz(exl::armv8::reg::W2, 0x1f)); // edit number of nerves
    exl::patch::CodePatcher(0x50eb64).BranchLinkInst((void*)&speedboot::prepareLayoutInitInfo);
}