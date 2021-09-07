#pragma once

#include <vector>
#include <memory>
#include "pivot/ecs/Component.hxx"

class Entity
{
public:
    Entity(std::string type, std::string name, Entity *parent): _type(type), _name(name), _parent(parent) {}
    void update()
    {
        for (auto &component: _components) component->update();
    }
    void draw()
    {
        for (auto &component: _components) component->draw();
    }
    bool isActive() const { return _active; }
    void destroy() { _active = false; }

    template <typename T>
    bool hasComponent() const
    {
        return _componentBitSet[getComponentTypeID<T>()];
    }

    template <typename T, typename... TArgs>
    T &addComponent(TArgs &&...mArgs)
    {
        T *component(new T(std::forward<TArgs>(mArgs)...));
        component->_entity = this;
        std::unique_ptr<Component> uPtr(component);
        _components.emplace_back(std::move(uPtr));
        _componentArray[getComponentTypeID<T>()] = component;
        _componentBitSet[getComponentTypeID<T>()] = true;
        component->init();
        return *component;
    }

    template <typename T>
    T &getComponent() const
    {
        auto ptr(_componentArray[getComponentTypeID<T>()]);
        return *static_cast<T *>(ptr);
    }

    std::string getType() { return _type; }

    std::string getName() { return _name; }

    Entity *getParent() { return _parent; }

private:
    bool _active = true;
    std::vector<std::unique_ptr<Component>> _components;
    ComponentArray _componentArray;
    ComponentBitSet _componentBitSet;
    std::string _type;
    std::string _name;
    Entity *_parent;
};