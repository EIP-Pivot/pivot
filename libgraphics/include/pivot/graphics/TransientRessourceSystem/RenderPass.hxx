#pragma once

#include "pivot/graphics/TransientRessourceSystem/RenderContext.hxx"
#include "pivot/graphics/TransientRessourceSystem/RenderPassBuilder.hxx"
#include "pivot/graphics/TransientRessourceSystem/RenderPassRessource.hxx"

#include <functional>

namespace pivot::graphics::trs
{

template <class T>
using SetupCallback = std::function<void(T &, RenderPassBuilder &)>;

template <typename T, typename D>
concept validSetupLambda = std::invocable<T, D &, RenderPassBuilder &> && std::is_convertible_v<T, SetupCallback<D>>;

template <class T>
using ExecutionCallBack = std::function<void(const T &, const RenderPassRessources &, const RenderContext &)>;

template <typename T, typename D>
concept validExecutionLambda = std::invocable<T, const D &, const RenderPassRessources &, const RenderContext &> &&
    std::is_convertible_v<T, ExecutionCallBack<D>>;

class IRenderPass
{
public:
    virtual void setup(RenderPassBuilder &builder) = 0;
    virtual void execution(const RenderPassRessources &renderRessource, const RenderContext &context) const = 0;
};

using RenderPassPtr = std::unique_ptr<IRenderPass>;

template <class T>
requires std::is_default_constructible_v<T>
class RenderPass final : public IRenderPass
{
public:
    RenderPass(SetupCallback<T> setup, ExecutionCallBack<T> execution): setup_c(setup), execution_c(execution) {}

    void setup(RenderPassBuilder &builder) override { return setup_c(data, builder); }

    void execution(const RenderPassRessources &renderRessource, const RenderContext &context) const override
    {
        return execution_c(data, renderRessource, context);
    }

public:
    T data = {};
    SetupCallback<T> setup_c;
    ExecutionCallBack<T> execution_c;
};

}    // namespace pivot::graphics::trs
