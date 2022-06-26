#pragma once

#include "pivot/exception.hxx"
#include "pivot/graphics/TransientRessourceSystem/RenderContext.hxx"
#include "pivot/graphics/TransientRessourceSystem/RenderPass.hxx"
#include "pivot/graphics/TransientRessourceSystem/RenderPassBuilder.hxx"

#include <memory>
#include <unordered_map>

namespace pivot::graphics::trs
{

class FrameGraph
{
public:
    RUNTIME_ERROR(FrameGraph)

public:
    FrameGraph(VulkanBase &base_ref, AssetStorage &storage);
    ~FrameGraph();

    template <class T, class S, class E>
    requires validSetupLambda<S, T> && validExecutionLambda<E, T> T &
    addCallbackPass(const std::string &name, S &&setupCallback, E &&executionCallback)
    {
        static_assert(sizeof(setupCallback) < 1000, "Setup callback is bigger than 1 KB");
        static_assert(sizeof(executionCallback) < 1000, "Execution callback is bigger than 1 KB");
        auto ren = std::make_unique<RenderPass<T>>(setupCallback, executionCallback);
        ren->setup(storage[name].second);
        auto &ret = *ren;
        storage.at(name).first = std::move(ren);
        return ret.data;
    }

    bool compile();

private:
    void fillFirstStage();

private:
    VulkanBase &base_ref;
    AssetStorage &assetStorage;
    std::map<std::string, std::pair<RenderPassPtr, RenderPassBuilder>> storage;
    std::vector<std::vector<std::pair<RenderPassPtr, RenderPassBuilder>>> compiledPasses;
};

}    // namespace pivot::graphics::trs
