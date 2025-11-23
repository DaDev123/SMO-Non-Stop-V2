#include <exlaunch.hpp>

#include "Sequence/HakoniwaSequence.h"
#include "System/GameDataFunction.h"
#include "System/GameSystem.h"
#include "Scene/StageScene.h"

// Noclip
#include "Library/Controller/InputFunction.h"
#include "Library/LiveActor/ActorFlagFunction.h"
#include "Player/PlayerActorHakoniwa.h"
#include "Library/Camera/CameraUtil.h"
#include "Library/Math/MathUtil.h"
#include "Library/LiveActor/ActorPoseUtil.h"
#include "Library/LiveActor/ActorMovementFunction.h"

static f32 speedGain = 0.0f;
static bool isNoclip = false;

void noclip(PlayerActorHakoniwa* player) {
    if (al::isPadTriggerUp(-1)) {
        isNoclip = !isNoclip;

        if (!isNoclip) {
            al::onCollide(player);
            player->endDemoPuppetable();
        } else {
            player->startDemoPuppetable();
        }
    }

    if (!isNoclip)
        return;

    f32 speed = 25.0f;
    f32 speedMax = 150.0f;
    f32 vspeed = 20.0f;

    sead::Vector3f* playerPos = al::getTransPtr(player);
    sead::Vector3f cameraPos = al::getCameraPos(player, 0);
    sead::Vector2f leftStick = al::getLeftStick(-1);

    al::offCollide(player);
    al::setVelocityZero(player);

    f32 d = sead::Mathf::sqrt(al::powerIn(playerPos->x - cameraPos.x, 2) +
                              (al::powerIn(playerPos->z - cameraPos.z, 2)));
    f32 vx = ((speed + speedGain) / d) * (playerPos->x - cameraPos.x);
    f32 vz = ((speed + speedGain) / d) * (playerPos->z - cameraPos.z);

    if (!al::isPadHoldZR(-1)) {
        playerPos->x -= leftStick.x * vz;
        playerPos->z += leftStick.x * vx;

        playerPos->x += leftStick.y * vx;
        playerPos->z += leftStick.y * vz;

        if (al::isPadHoldX(-1))
            speedGain -= 0.5f;

        if (al::isPadHoldY(-1))
            speedGain += 0.5f;

        if (speedGain <= 0.0f)
            speedGain = 0.0f;

        if (speedGain >= speedMax)
            speedGain = speedMax;

        if (al::isPadHoldZL(-1) || al::isPadHoldA(-1))
            playerPos->y -= (vspeed + speedGain / 6);

        if (al::isPadHoldB(-1))
            playerPos->y += (vspeed + speedGain / 6);
    }
}

HOOK_DEFINE_TRAMPOLINE(StageSceneControlHook) {
    static void Callback(StageScene* scene, const al::ActorInitInfo& info) {
        GameDataHolderWriter* holder = static_cast<GameDataHolderWriter*>(
            &((HakoniwaSequence*)GameSystemFunction::getGameSystem()->mSequence)
                 ->mGameDataHolderAccessor);

        GameDataFunction::enableCap(*holder);

        al::LiveActor* player = rs::getPlayerActor(scene);
        if (player)
            GameDataFunction::talkCapNearHomeInWaterfall(player);
            noclip((PlayerActorHakoniwa*)rs::getPlayerActor(scene));

        Orig(scene, info);
    }
};

void NonStopHooks() {
    StageSceneControlHook::InstallAtSymbol("_ZN10StageScene7controlEv");
}