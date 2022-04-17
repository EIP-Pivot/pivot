#pragma once

namespace pivot::graphics
{

/// Indicated the number of frame rendered in advance by the engine
constexpr const auto MaxFrameInFlight = 3;
static_assert(MaxFrameInFlight >= 1);

}    // namespace pivot::graphics
