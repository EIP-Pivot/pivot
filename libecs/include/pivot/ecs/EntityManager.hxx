#pragma once

#include "pivot/ecs/Entity.hxx"

class EntityManager
{
public:
    void update()
    {
        for (auto &entity: _entities) entity->update();
    }

    void draw()
    {
        for (auto &entity: _entities) entity->draw();
    }

    void refresh()
    {
        _entities.erase(std::remove_if(std::begin(_entities), std::end(_entities),
                                       [](const Entity *mEntity) { return !mEntity->isActive(); }),
                        std::end(_entities));
    }

    Entity &addEntity(std::string type, std::string name, Entity *parent = nullptr)
    {
        Entity *entity = new Entity(type, name, parent);
        _entities.emplace_back(entity);
        return *entity;
    }

    std::vector<Entity *> getEntities() { return _entities; }

private:
    std::vector<Entity *> _entities;
};