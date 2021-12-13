#include <pivot/ecs/Core/Systems/description.hxx>
#include <pivot/ecs/Core/Systems/function_traits.hxx>
#include <stdexcept>

namespace pivot::ecs::systems
{

void Description::validate() const
{
    if (this->name.empty()) { throw ValidationError("Empty component name"); }
}

int coucou(std::string oui, std::size_t i)
{
    return 0;
}
Description Description::build_system_description(pivot::ecs::component::GlobalIndex globalIndex, const std::string &name, auto system)
{
    Description newSystem;
    newSystem.name = name;

    int (*fun_ptr)(std::string, std::size_t) = &coucou;

    typedef function_traits<decltype(fun_ptr)> traits;

    for (auto i = 0; i < traits::arity; i++) {
        auto argName = typeid(traits::arg<0>::type).name();
        newSystem.arguments.push_back(std::string(argName));
    }
    return newSystem;
}

}    // namespace pivot::ecs::component