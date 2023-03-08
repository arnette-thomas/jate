// #include <iostream>
#include <jate/application.h>

#include <jate/components/render_units/rect2d_render_unit.h>

int main(int argc, char** argv)
{
    // Hello, world
    jate::Application app;
    auto world = app.createWorld();

    auto rectangle = world->spawnEntity();
    rectangle.addComponent<jate::components::Rect2DRenderUnit>();

    app.run();
    return EXIT_SUCCESS;
}