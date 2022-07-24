#include <boost/fusion/include/adapt_struct.hpp>

#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/description_helpers_impl.hxx>

#include <pivot/builtins/components/Text.hxx>

using namespace pivot::builtins::components;

BOOST_FUSION_ADAPT_STRUCT(Text, content, color);
PIVOT_REGISTER_COMPONENT(Text, DenseTypedComponentArray<Text>);
