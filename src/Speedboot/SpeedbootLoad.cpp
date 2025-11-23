#include "Speedboot/SpeedbootLoad.hpp"

#include <gfx/seadColor.h>
#include <math/seadMathCalcCommon.h>
#include <prim/seadSafeString.h>

#include "Library/Layout/LayoutActionFunction.h"
#include "Library/Layout/LayoutActor.h"
#include "Library/Layout/LayoutActorUtil.h"
#include "Library/Layout/LayoutInitInfo.h"
#include "Library/Nerve/NerveSetupUtil.h"
#include "Library/Nerve/NerveUtil.h"

#include "Sequence/WorldResourceLoader.h"

namespace {
    NERVE_IMPL(SpeedbootLoad, Appear)
    NERVE_IMPL(SpeedbootLoad, Wait)
    NERVE_IMPL(SpeedbootLoad, Decrease)
    NERVE_IMPL(SpeedbootLoad, End)

    NERVES_MAKE_NOSTRUCT(SpeedbootLoad, Appear, Wait, Decrease, End)
} // namespace

SpeedbootLoad::SpeedbootLoad(WorldResourceLoader* resourceLoader, const al::LayoutInitInfo& initInfo, const char* stageName)
    : al::LayoutActor("SpeedbootLoad"), worldResourceLoader(resourceLoader), mStageName(stageName) {
    al::initLayoutActor(this, initInfo, "SpeedbootLoad", nullptr);
    initNerve(&Appear, 0);
    // al::setPaneLocalScale(this, "All", { 5.0f, 5.0f });

    LayoutActor::appear();
}

void SpeedbootLoad::exeAppear() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "Appear", nullptr);
    }

    if (al::isActionEnd(this, nullptr)) {
        al::setNerve(this, &Wait);
    }
}

void SpeedbootLoad::exeWait() {
    if (al::isActionEnd(this, nullptr)) {
        al::setNerve(this, &Decrease);
    }
}

void SpeedbootLoad::exeDecrease() {
    mProgression = worldResourceLoader->calcLoadPercent() / 100.0f;
    sead::Color4u8 color = sead::Color4u8::cYellow;

    color.r = 255 - sead::Mathi::ceil((s32)mProgression * 255);
    color.g = sead::Mathi::ceil((s32)mProgression * 255);

    mRotTime += 0.03f;
    f32 rotation = cosf(mRotTime) * 5;

    // Convert stage name to kingdom name
    const char16_t* kingdomName = u"Unknown Kingdom";
    if (mStageName) {
        // Compare stage names and set appropriate kingdom names
        if (strcmp(mStageName, "CapWorldHomeStage") == 0) {
            kingdomName = u"Cap Kingdom";
        } else if (strcmp(mStageName, "WaterfallWorldHomeStage") == 0) {
            kingdomName = u"Cascade Kingdom";
        } else if (strcmp(mStageName, "SandWorldHomeStage") == 0) {
            kingdomName = u"Sand Kingdom";
        } else if (strcmp(mStageName, "ForestWorldHomeStage") == 0) {
            kingdomName = u"Wooded Kingdom";
        } else if (strcmp(mStageName, "LakeWorldHomeStage") == 0) {
            kingdomName = u"Lake Kingdom";
        } else if (strcmp(mStageName, "CloudWorldHomeStage") == 0) {
            kingdomName = u"Cloud Kingdom";
        } else if (strcmp(mStageName, "ClashWorldHomeStage") == 0) {
            kingdomName = u"Lost Kingdom";
        } else if (strcmp(mStageName, "CityWorldHomeStage") == 0) {
            kingdomName = u"Metro Kingdom";
        } else if (strcmp(mStageName, "SnowWorldHomeStage") == 0) {
            kingdomName = u"Snow Kingdom";
        } else if (strcmp(mStageName, "SeaWorldHomeStage") == 0) {
            kingdomName = u"Seaside Kingdom";
        } else if (strcmp(mStageName, "LavaWorldHomeStage") == 0) {
            kingdomName = u"Luncheon Kingdom";
        } else if (strcmp(mStageName, "BossRaidWorldHomeStage") == 0) {
            kingdomName = u"Ruined Kingdom";
        } else if (strcmp(mStageName, "SkyWorldHomeStage") == 0) {
            kingdomName = u"Bowser's Kingdom";
        } else if (strcmp(mStageName, "MoonWorldHomeStage") == 0) {
            kingdomName = u"Moon Kingdom";
        } else if (strcmp(mStageName, "PeachWorldHomeStage") == 0) {
            kingdomName = u"Mushroom Kingdom";
        } else if (strcmp(mStageName, "Special1WorldHomeStage") == 0) {
            kingdomName = u"Dark Side";
        } else if (strcmp(mStageName, "Special2WorldHomeStage") == 0) {
            kingdomName = u"Darker Side";
        } else {
            // For unknown stages, convert the stage name to wide string
            sead::WFormatFixedSafeString<0x100> fallbackString(u"");
            for (const char* c = mStageName; *c != '\0'; c++) {
                fallbackString.appendWithFormat(u"%c", (char16_t)(*c));
            }
            al::setPaneString(this, "TxtKingdom", fallbackString.cstr(), 0);
            goto skip_kingdom_set;
        }
    }
    
    al::setPaneString(this, "TxtKingdom", kingdomName, 0);
    skip_kingdom_set:

    // Animated "Loading" text with dots (slower speed)
    s32 dotCount = ((s32)(mRotTime * 0.8f)) % 4; // Cycles through 0, 1, 2, 3 (slower)
    sead::WFormatFixedSafeString<0x20> loadingString(u"Loading");
    for (s32 i = 0; i < dotCount; i++) {
        loadingString.append(u".");
    }
    al::setPaneString(this, "TxtLoading", loadingString.cstr(), 0);

    // Debug info
    sead::WFormatFixedSafeString<0x100> debugString(
        u"Progress: %.1f%%\nTime: %.02f\nRot: %.02f", 
        mProgression * 100.0f, mRotTime, rotation
    );
    al::setPaneString(this, "TxtDebug", debugString.cstr(), 0);
    al::setPaneString(this, "TxtCurModName", u"Non-Stop V2", 0);

    if (mProgression < 1.0) {
        // al::startFreezeAction(this, "Decrease", 60.f * (1.0f - actual), "State");
        al::setPaneLocalScale(this, "PicBar", {mProgression, 2.0f});
        //al::setPaneVtxColor(this, "PicBar", color);

        al::setPaneLocalScale(this, "PicBarFill", {30.f, 1.0f});
        //al::setPaneVtxColor(this, "PicBarFill", color);

        //al::setPaneLocalTrans(this, "PicPointer", {(mProgression * 1280.0f) - 640.0f, -260.0f});
        //al::setPaneLocalRotate(this, "PicPointer", {0.0f, 0.0f, rotation});
        al::setPaneLocalRotate(this, "BloodMoon", {0.0f, 0.0f, rotation});
        al::setPaneLocalRotate(this, "BloodMoonStar", {0.0f, 0.0f, mRotTime * -15.0f}); // Much faster rotation

        al::setPaneLocalRotate(this, "PicBG", {0.0f, 0.0f, mRotTime * -3.0f});
    }

    if (mProgression > 1.0f) {
        al::setNerve(this, &End);
    }
}

void SpeedbootLoad::exeEnd() {
    if (al::isFirstStep(this)) {
        al::startAction(this, "End", nullptr);
    }

    if (al::isActionEnd(this, nullptr)) {
        kill();
    }
}