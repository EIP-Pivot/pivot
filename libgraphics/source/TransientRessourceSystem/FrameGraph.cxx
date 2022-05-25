#include "pivot/graphics/TransientRessourceSystem/FrameGraph.hxx"

namespace pivot::graphics::trs
{

FrameGraph::FrameGraph(VulkanBase &base_ref, AssetStorage &storage): base_ref(base_ref), assetStorage(storage) {}

FrameGraph::~FrameGraph() {}

}    // namespace pivot::graphics::trs
