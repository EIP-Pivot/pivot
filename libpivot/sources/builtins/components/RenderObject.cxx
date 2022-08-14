#include "pivot/builtins/components/RenderObject.hxx"
#include "pivot/ecs/Core/Component/DenseComponentArray.hxx"
#include "pivot/ecs/Core/Component/description_helpers_impl.hxx"
#include "pivot/graphics/types/Transform.hxx"

#include <boost/fusion/include/adapt_struct.hpp>

using namespace pivot::builtins::components;

namespace
{
struct RenderObjectAsset {
    RenderObjectAsset() = default;

    RenderObjectAsset(const pivot::graphics::RenderObject &ro)
        : meshID({ro.meshID}), pipelineID(ro.pipelineID), materialIndex({ro.materialIndex})
    {
    }

    pivot::graphics::RenderObject toRenderObject() const
    {
        return RenderObject{this->meshID.name, this->pipelineID, this->materialIndex.name};
    }

    pivot::ecs::data::Asset meshID;
    std::string pipelineID = "";
    pivot::ecs::data::Asset materialIndex;
};
}    // namespace

BOOST_FUSION_ADAPT_STRUCT(RenderObjectAsset, meshID, pipelineID, materialIndex);

namespace pivot::ecs::component::helpers
{

template struct Helpers<RenderObjectAsset>;

template <>
struct Helpers<pivot::graphics::RenderObject> {

    static data::Type getType() { return Helpers<RenderObjectAsset>::getType(); }

    static data::Value createValueFromType(const pivot::graphics::RenderObject &v)
    {
        return Helpers<RenderObjectAsset>::createValueFromType(v);
    }

    static void updateTypeWithValue(pivot::graphics::RenderObject &data, const data::Value &value)
    {
        RenderObjectAsset roa;
        Helpers<RenderObjectAsset>::updateTypeWithValue(roa, value);
        data = roa.toRenderObject();
    }
};

}    // namespace pivot::ecs::component::helpers

BOOST_FUSION_ADAPT_STRUCT(pivot::graphics::RenderObject, meshID, pipelineID, materialIndex);
PIVOT_REGISTER_WRAPPED_COMPONENT(RenderObject, pivot::graphics::RenderObject,
                                 DenseTypedComponentArray<pivot::graphics::RenderObject>);
