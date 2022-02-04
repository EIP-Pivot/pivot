#include <pivot/ecs/Core/Component/combination.hxx>

namespace pivot::ecs::component
{

Entity ArrayCombination::maxEntity() const
{
    return std::ranges::max_element(m_arrays, {}, [](auto &array) { return array.get().maxEntity(); })
        ->get()
        .maxEntity();
}

bool ArrayCombination::entityHasValue(Entity entity) const
{
    return std::ranges::all_of(m_arrays, [=](auto &array) { return array.get().entityHasValue(entity); });
}
}    // namespace pivot::ecs::component
