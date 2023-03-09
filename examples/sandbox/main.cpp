// #include <iostream>
#include <jate/application.h>

#include <jate/components/render_units/rect2d_render_unit.h>

int main(int argc, char** argv)
{
    // Hello, world
    jate::Application app;
    auto world = app.createWorld();

    auto rectangle = world->spawnEntity();
    auto rectRenderUnit = rectangle->addComponent<jate::components::Rect2DRenderUnit>();

    rectRenderUnit->setRect(0.f, 0.f, 0.5f, 0.3f);

    app.run();
    return EXIT_SUCCESS;
}