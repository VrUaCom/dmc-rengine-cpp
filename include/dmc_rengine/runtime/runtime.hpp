#pragma once

/// Umbrella header for the DMC Rengine runtime host layer.
///
/// The runtime is a platform/render/loop host. It reads every resource through
/// GDSpaces and mirrors Stage Ops scene state; it owns neither.

#include "dmc_rengine/runtime/application.hpp"
#include "dmc_rengine/runtime/frame_clock.hpp"
#include "dmc_rengine/runtime/headless_platform.hpp"
#include "dmc_rengine/runtime/null_render_device.hpp"
#include "dmc_rengine/runtime/platform.hpp"
#include "dmc_rengine/runtime/platform_event.hpp"
#include "dmc_rengine/runtime/render_backend_registry.hpp"
#include "dmc_rengine/runtime/render_device.hpp"
#include "dmc_rengine/runtime/resource_bridge.hpp"
#include "dmc_rengine/runtime/runtime_info.hpp"
#include "dmc_rengine/runtime/stage_host.hpp"
#include "dmc_rengine/runtime/status.hpp"
