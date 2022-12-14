
#include "vengine.h"

class LoadingScene : public Scene
{
private:
    uint32_t loadingTextureIndex;
    uint32_t blackTextureIndex;

    bool oneFrameHasPassed;

public:
    LoadingScene();
    virtual ~LoadingScene();

    // Inherited via Scene
    virtual void init() override;
    virtual void start() override;
    virtual void update() override;
    virtual void onTriggerStay(Entity e1, Entity e2) override;
    virtual void onTriggerEnter(Entity e1, Entity e2) override;
    virtual void onCollisionEnter(Entity e1, Entity e2) override;
    virtual void onCollisionStay(Entity e1, Entity e2) override;
    virtual void onCollisionExit(Entity e1, Entity e2) override;
};