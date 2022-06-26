#include "pivot/graphics/TransientRessourceSystem/FrameGraph.hxx"

namespace pivot::graphics::trs
{

FrameGraph::FrameGraph(VulkanBase &base_ref, AssetStorage &storage): base_ref(base_ref), assetStorage(storage) {}

FrameGraph::~FrameGraph() {}

bool FrameGraph::compile()
try {
    fillFirstStage();
    return true;
} catch (const FrameGraphError &fge) {
    logger.err("Frame Graph") << fge.what();
    return false;
}

void FrameGraph::fillFirstStage()
{
    compiledPasses.resize(1);
    for (const auto &[name, i]: storage) {
        if (!(i.second.readTicket.empty() && i.second.writeTicketConsumed.empty())) continue;

        compiledPasses.at(0).push_back(std::move(storage.at(name)));
        storage.erase(name);
    }
    if (compiledPasses.at(0).empty()) throw FrameGraphError("Impossible to find the first passes");
}

}    // namespace pivot::graphics::trs
