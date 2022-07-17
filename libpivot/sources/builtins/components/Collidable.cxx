#include <boost/fusion/include/adapt_struct.hpp>

#include <pivot/builtins/components/Collidable.hxx>
#include <pivot/ecs/Core/Component/FlagComponentStorage.hxx>
#include <pivot/ecs/Core/Component/description_helpers_impl.hxx>

using namespace pivot::builtins::components;
using namespace pivot::ecs::data;
using namespace pivot::ecs;

namespace
{
std::unique_ptr<component::IComponentArray> createContainer(component::Description description)
{
    return std::make_unique<component::FlagComponentStorage>(description);
}
}    // namespace
const component::Description Collidable::description = {.name = "Collidable",
                                                        .type = Type{BasicType::Void},
                                                        .provenance = Provenance::builtin(),
                                                        .defaultValue = Value{Void{}},
                                                        .createContainer = createContainer};
