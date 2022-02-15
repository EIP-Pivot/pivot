#include <boost/fusion/include/adapt_struct.hpp>
#include <pivot/ecs/Core/Component/DenseComponentArray.hxx>
#include <pivot/ecs/Core/Component/description_helpers_impl.hxx>
#include <pivot/graphics/types/RenderObject.hxx>

struct TransformWrapper {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

BOOST_FUSION_ADAPT_STRUCT(TransformWrapper, position, rotation, scale);

namespace pivot::ecs::component::helpers
{
template <>
struct Helpers<Transform> {
    static data::Type getType() { return Helpers<TransformWrapper>::getType(); }
    static data::Value createValueFromType(const Transform &v)
    {
        TransformWrapper wrapper{v.getPosition(), v.getRotation(), v.getScale()};
        return Helpers<TransformWrapper>::createValueFromType(wrapper);
    }
    static void updateTypeWithValue(Transform &data, const data::Value &value)
    {
        TransformWrapper wrapper;
        Helpers<TransformWrapper>::updateTypeWithValue(wrapper, value);
        data.setPosition(wrapper.position);
        data.setRotation(wrapper.rotation);
        data.setScale(wrapper.scale);
    }
};
}    // namespace pivot::ecs::component::helpers

BOOST_FUSION_ADAPT_STRUCT(RenderObject::ObjectInformation, transform, textureIndex, materialIndex);
BOOST_FUSION_ADAPT_STRUCT(RenderObject, meshID, objectInformation);
PIVOT_REGISTER_COMPONENT(RenderObject, DenseTypedComponentArray<RenderObject>);
