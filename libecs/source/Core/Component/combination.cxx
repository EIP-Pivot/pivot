#include <pivot/ecs/Core/Component/combination.hxx>

namespace pivot::ecs::component
{

Entity ArrayCombination::maxEntity() const
{
    if (m_arrays.empty()) return 0;
    return std::ranges::max_element(m_arrays, {}, [](auto &array) { return array.get().maxEntity(); })
        ->get()
        .maxEntity();
}

bool ArrayCombination::entityHasValue(Entity entity) const
{
    // Empty combinations should iterate only once, on the entity 0
    if (m_arrays.empty()) return (entity == 0);

    return std::ranges::all_of(m_arrays, [=](auto &array) { return array.get().entityHasValue(entity); });
}
}    // namespace pivot::ecs::component
