#include "pivot/builtins/components/RenderObject.hxx"
#include "pivot/ecs/Core/Component/DenseComponentArray.hxx"
#include "pivot/ecs/Core/Component/description_helpers_impl.hxx"
#include "pivot/graphics/types/Transform.hxx"

#include <boost/fusion/include/adapt_struct.hpp>

using namespace pivot::builtins::components;
using namespace pivot::ecs::data;
using Transform = pivot::graphics::Transform;

BOOST_FUSION_ADAPT_STRUCT(Transform, position, rotation, scale);

template struct pivot::ecs::component::helpers::Helpers<Transform>;

namespace
{
struct RenderObjectAsset {
    RenderObjectAsset() = default;

    Asset meshID;
    std::string pipelineID = "";
    Asset materialIndex;
    Transform transform;

    RenderObjectAsset(const RenderObject &ro)
        : meshID({ro.meshID}), pipelineID(ro.pipelineID), materialIndex({ro.materialIndex}), transform(ro.transform)
    {
    }

    RenderObject toRenderObject() const
    {
        return RenderObject{this->meshID.name, this->pipelineID, this->materialIndex.name, this->transform};
    }
};
}    // namespace

BOOST_FUSION_ADAPT_STRUCT(RenderObjectAsset, meshID, pipelineID, materialIndex, transform);

namespace pivot::ecs::component::helpers
{

template struct Helpers<RenderObjectAsset>;

template <>
struct Helpers<RenderObject> {

    static data::Type getType() { return Helpers<RenderObjectAsset>::getType(); }

    static data::Value createValueFromType(const RenderObject &v)
    {
        return Helpers<RenderObjectAsset>::createValueFromType(v);
    }

    static void updateTypeWithValue(RenderObject &data, const data::Value &value)
    {
        RenderObjectAsset roa;
        Helpers<RenderObjectAsset>::updateTypeWithValue(roa, value);
        data = roa.toRenderObject();
    }
};

}    // namespace pivot::ecs::component::helpers

BOOST_FUSION_ADAPT_STRUCT(RenderObject, meshID, pipelineID, materialIndex, transform);
PIVOT_REGISTER_COMPONENT(RenderObject, DenseTypedComponentArray<RenderObject>);
