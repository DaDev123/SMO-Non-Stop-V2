#include "Library/Layout/LayoutActor.h"

namespace al {
    class LayoutInitInfo;
}

class WorldResourceLoader;

class SpeedbootLoad : public al::LayoutActor {
public:
    SpeedbootLoad(WorldResourceLoader* resourceLoader, const al::LayoutInitInfo& initInfo, const char* stageName);

    void exeAppear();
    void exeWait();
    void exeDecrease();
    void exeEnd();

    f32 mProgression = 0.0f;
    f32 mRotTime = 0.0f;

private:
    WorldResourceLoader* worldResourceLoader;
    const char* mStageName;
};
