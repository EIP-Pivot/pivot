#pragma once

#include "pivot/graphics/TransientRessourceSystem/RenderContext.hxx"
#include "pivot/graphics/TransientRessourceSystem/RenderPassBuilder.hxx"

#include <functional>

namespace pivot::graphics::trs
{

template <class T>
using SetupCallback = std::function<void(T &, RenderPassBuilder &)>;

template <typename T, typename D>
concept validSetupLambda = std::invocable<T, D &, RenderPassBuilder &> && std::is_convertible_v<T, SetupCallback<D>>;

template <class T>
using ExecutionCallBack = std::function<void(const T &, const RenderPassBuilder &, RenderContext &)>;

template <typename T, typename D>
concept validExecutionLambda = std::invocable<T, const D &, const RenderPassBuilder &, RenderContext &> &&
    std::is_convertible_v<T, ExecutionCallBack<D>>;

class IRenderPass
{
public:
    virtual void setup() = 0;
    virtual void execution(RenderContext &context) const = 0;
};

using RenderPassPtr = std::unique_ptr<IRenderPass>;

template <class T>
requires std::is_default_constructible_v<T>
class RenderPass final : public IRenderPass
{
public:
    RenderPass(RenderPassBuilder builder, SetupCallback<T> setup, ExecutionCallBack<T> execution)
        : builder(std::move(builder)), setup_c(setup), execution_c(execution)
    {
    }

    void setup() override { return setup_c(data, builder); }

    void execution(RenderContext &context) const override { return execution_c(data, builder, context); }

public:
    RenderPassBuilder builder;
    T data = {};
    SetupCallback<T> setup_c;
    ExecutionCallBack<T> execution_c;
};

}    // namespace pivot::graphics::trs
