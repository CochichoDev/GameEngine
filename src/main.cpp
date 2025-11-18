#include "world.hpp"

#include "entity.hpp"

EntityID EntityManager::s_entity_counter = 0;

int main() {
    World world;

    EntityID player = world.create_entity();
    Transform player_transform;
    world.add_component(player, player_transform);

    RectangleDrawable player_renderable(world.get_component_idx<Transform>(player).value());
    world.add_component(player, player_renderable);

    world.run();

    return 0;
}
