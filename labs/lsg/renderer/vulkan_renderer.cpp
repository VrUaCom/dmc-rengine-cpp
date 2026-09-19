#include "vulkan_renderer.hpp"
#include "rengine/lsg/derived_character.hpp"
#include "rengine/lsg/derived_eye.hpp"
#include "rengine/lsg/genome.hpp"
#include "rengine/lsg/eye_runtime.hpp"
#include "rengine/lsg/lighting_runtime.hpp"
#include "rengine/lsg/projection.hpp"
#include "rengine/lsg/rmesh.hpp"

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <vulkan/vulkan.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#if defined(__ANDROID__)
#include <android/asset_manager.h>
#include <android/native_window.h>
#include <vulkan/vulkan_android.h>
#endif

namespace rengine::lsg {

inline constexpr std::uint32_t kShadowMapSize = 2048u;

struct VulkanRenderer::Impl {
  VkInstance instance{VK_NULL_HANDLE};
  VkSurfaceKHR surface{VK_NULL_HANDLE};
  VkPhysicalDevice physical{VK_NULL_HANDLE};
  VkDevice device{VK_NULL_HANDLE};
  VkQueue queue{VK_NULL_HANDLE};
  std::uint32_t queue_family{};
  VkSwapchainKHR swapchain{VK_NULL_HANDLE};
  VkFormat format{VK_FORMAT_UNDEFINED};
  VkExtent2D window_extent{};
  VkExtent2D swapchain_extent{};
  Extent2u logical_extent{};
  std::uint32_t surface_rotation{};
  std::vector<VkImage> images;
  std::vector<VkImageView> views;
  VkFormat depth_format{VK_FORMAT_UNDEFINED};
  VkImage depth_image{VK_NULL_HANDLE};
  VkDeviceMemory depth_memory{VK_NULL_HANDLE};
  VkImageView depth_view{VK_NULL_HANDLE};
  VkRenderPass render_pass{VK_NULL_HANDLE};
  std::vector<VkFramebuffer> framebuffers;
  VkFormat shadow_depth_format{VK_FORMAT_UNDEFINED};
  VkImage shadow_depth_image{VK_NULL_HANDLE};
  VkDeviceMemory shadow_depth_memory{VK_NULL_HANDLE};
  VkImageView shadow_depth_view{VK_NULL_HANDLE};
  VkSampler shadow_sampler{VK_NULL_HANDLE};
  VkRenderPass shadow_render_pass{VK_NULL_HANDLE};
  VkFramebuffer shadow_framebuffer{VK_NULL_HANDLE};
  VkImage self_shadow_depth_image{VK_NULL_HANDLE};
  VkDeviceMemory self_shadow_depth_memory{VK_NULL_HANDLE};
  VkImageView self_shadow_depth_view{VK_NULL_HANDLE};
  VkFramebuffer self_shadow_framebuffer{VK_NULL_HANDLE};
  VkPipeline shadow_pipeline{VK_NULL_HANDLE};
  VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
  VkPipeline pipeline{VK_NULL_HANDLE};
  VkPipelineLayout eye_pipeline_layout{VK_NULL_HANDLE};
  VkPipeline eye_inner_pipeline{VK_NULL_HANDLE};
  VkPipeline eye_cornea_pipeline{VK_NULL_HANDLE};
  VkDescriptorSetLayout frame_lighting_set_layout{VK_NULL_HANDLE};
  VkDescriptorPool frame_lighting_descriptor_pool{VK_NULL_HANDLE};
  VkDescriptorSet frame_lighting_descriptor_set{VK_NULL_HANDLE};
  VkBuffer frame_lighting_buffer{VK_NULL_HANDLE};
  VkDeviceMemory frame_lighting_memory{VK_NULL_HANDLE};
  void* frame_lighting_mapped{};
  struct ProfileMeshGpu {
    VkBuffer vertex_buffer{VK_NULL_HANDLE};
    VkDeviceMemory vertex_memory{VK_NULL_HANDLE};
    VkBuffer index_buffer{VK_NULL_HANDLE};
    VkDeviceMemory index_memory{VK_NULL_HANDLE};
    std::uint32_t index_count{};
    std::array<float, 3> mesh_center{};
    float meters_per_unit{1.0f};
  };
  std::array<ProfileMeshGpu, 2> profile_meshes{};
  std::array<ProfileMeshGpu, 2> eye_meshes{};
  CameraController camera{};
  DiagnosticRenderMode diagnostic_mode{DiagnosticRenderMode::genome_perspective};
  SurfaceDiagnosticMode surface_diagnostic_mode{SurfaceDiagnosticMode::none};
  int ui_tooltip_row{-1};
  PhysiologyPreset physiology_preset{PhysiologyPreset::normal};
  EyeDiagnosticMode eye_diagnostic_mode{EyeDiagnosticMode::normal};
  LightingRuntimeState lighting{lighting_for(LightingPreset::noon, OpticalFilterPreset::clear)};
  std::array<EyeRuntimeState, 2> eye_runtime{};
  float last_eye_time_seconds{};
  std::uint32_t last_profile_index{};
  VkCommandPool command_pool{VK_NULL_HANDLE};
  std::vector<VkCommandBuffer> command_buffers;
  VkSemaphore image_available{VK_NULL_HANDLE};
  VkSemaphore render_finished{VK_NULL_HANDLE};
  VkFence in_flight{VK_NULL_HANDLE};
  std::uint64_t estimated_bytes{};
  bool initialized{};
};

namespace {
struct GpuVertex {
  float position[3]{};
  float normal[3]{};
  float uv[2]{};
  std::uint32_t region{};
};
static_assert(sizeof(GpuVertex) == 36);

struct PushConstants {
  float center_units[4]{};
  float camera[4]{};
  float geometry0[4]{};
  float geometry1[4]{};
  float skin0[4]{};
  float micro0[4]{};
  float render[4]{};
  std::uint32_t flags[4]{};
};
static_assert(sizeof(PushConstants) == 128);

struct EyePushConstants {
  float center_units[4]{};
  float camera[4]{};
  float geometry0[4]{};
  float geometry1[4]{};
  float render[4]{};
  float eye0[4]{}; // primary iris rgb, pupil bias
  float eye1[4]{}; // secondary iris rgb, sclera tint
  std::uint32_t flags[4]{}; // rotation, profile, vascularity byte, eye seed low
};
static_assert(sizeof(EyePushConstants) == 128);

enum class EyeRenderPass : std::uint32_t {
  inner = 0,
  cornea = 1,
};
static_assert(static_cast<std::uint32_t>(EyeRenderPass::inner) == 0u);
static_assert(static_cast<std::uint32_t>(EyeRenderPass::cornea) == 1u);

struct EyePipelineStateContract {
  VkBool32 blend_enable;
  VkBool32 depth_write_enable;
};

constexpr EyePipelineStateContract kInnerEyePipelineState{VK_FALSE, VK_TRUE};
constexpr EyePipelineStateContract kCorneaEyePipelineState{VK_TRUE, VK_FALSE};
static_assert(kInnerEyePipelineState.blend_enable == VK_FALSE);
static_assert(kInnerEyePipelineState.depth_write_enable == VK_TRUE);
static_assert(kCorneaEyePipelineState.blend_enable == VK_TRUE);
static_assert(kCorneaEyePipelineState.depth_write_enable == VK_FALSE);

struct alignas(16) FrameLightingGpu {
  float sun_direction_intensity[4]{};
  float sun_tint_sky_intensity[4]{};
  float sky_zenith_exposure[4]{};
  float sky_horizon_scene_lum[4]{};
  float filter_tint_transmission[4]{};
  float eye_filter_misc[4]{};
  std::uint32_t modes[4]{};
};
static_assert(alignof(FrameLightingGpu) == 16);
static_assert(sizeof(FrameLightingGpu) == 112);
static_assert(offsetof(FrameLightingGpu, sun_direction_intensity) == 0);
static_assert(offsetof(FrameLightingGpu, sun_tint_sky_intensity) == 16);
static_assert(offsetof(FrameLightingGpu, sky_zenith_exposure) == 32);
static_assert(offsetof(FrameLightingGpu, sky_horizon_scene_lum) == 48);
static_assert(offsetof(FrameLightingGpu, filter_tint_transmission) == 64);
static_assert(offsetof(FrameLightingGpu, eye_filter_misc) == 80);
static_assert(offsetof(FrameLightingGpu, modes) == 96);

FrameLightingGpu make_frame_lighting_gpu(const LightingRuntimeState& lighting) noexcept {
  FrameLightingGpu gpu{};
  gpu.sun_direction_intensity[0] = lighting.sun_direction[0];
  gpu.sun_direction_intensity[1] = lighting.sun_direction[1];
  gpu.sun_direction_intensity[2] = lighting.sun_direction[2];
  gpu.sun_direction_intensity[3] = lighting.direct_sun_intensity;

  gpu.sun_tint_sky_intensity[0] = lighting.sun_tint[0];
  gpu.sun_tint_sky_intensity[1] = lighting.sun_tint[1];
  gpu.sun_tint_sky_intensity[2] = lighting.sun_tint[2];
  gpu.sun_tint_sky_intensity[3] = lighting.sky_intensity;

  gpu.sky_zenith_exposure[0] = lighting.sky_zenith_tint[0];
  gpu.sky_zenith_exposure[1] = lighting.sky_zenith_tint[1];
  gpu.sky_zenith_exposure[2] = lighting.sky_zenith_tint[2];
  gpu.sky_zenith_exposure[3] = lighting.exposure;

  gpu.sky_horizon_scene_lum[0] = lighting.sky_horizon_tint[0];
  gpu.sky_horizon_scene_lum[1] = lighting.sky_horizon_tint[1];
  gpu.sky_horizon_scene_lum[2] = lighting.sky_horizon_tint[2];
  gpu.sky_horizon_scene_lum[3] = lighting.scene_luminance;

  gpu.filter_tint_transmission[0] = lighting.filter_tint[0];
  gpu.filter_tint_transmission[1] = lighting.filter_tint[1];
  gpu.filter_tint_transmission[2] = lighting.filter_tint[2];
  gpu.filter_tint_transmission[3] = lighting.filter_transmission;

  gpu.eye_filter_misc[0] = lighting.effective_eye_luminance;
  gpu.eye_filter_misc[1] = lighting.polarization_strength;
  gpu.modes[0] = static_cast<std::uint32_t>(lighting.preset);
  gpu.modes[1] = static_cast<std::uint32_t>(lighting.filter);
  return gpu;
}

const char* platform_surface_extension() noexcept {
#if defined(__ANDROID__)
  return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#elif defined(_WIN32)
  return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#else
  return nullptr;
#endif
}

std::uint32_t surface_rotation_code(VkSurfaceTransformFlagBitsKHR transform) noexcept {
  if ((transform & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) != 0u) return 1u;
  if ((transform & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) != 0u) return 2u;
  if ((transform & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) != 0u) return 3u;
  return 0u;
}

bool create_platform_surface(VkInstance instance, void* native_window, VkSurfaceKHR& surface) {
#if defined(__ANDROID__)
  VkAndroidSurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
  info.window = static_cast<ANativeWindow*>(native_window);
  return info.window != nullptr && vkCreateAndroidSurfaceKHR(instance, &info, nullptr, &surface) == VK_SUCCESS;
#elif defined(_WIN32)
  VkWin32SurfaceCreateInfoKHR info{VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
  info.hinstance = GetModuleHandleW(nullptr);
  info.hwnd = static_cast<HWND>(native_window);
  return info.hinstance != nullptr && info.hwnd != nullptr &&
         vkCreateWin32SurfaceKHR(instance, &info, nullptr, &surface) == VK_SUCCESS;
#else
  (void)instance; (void)native_window; (void)surface; return false;
#endif
}

VkExtent2D native_window_extent(void* native_window) noexcept {
#if defined(__ANDROID__)
  auto* window = static_cast<ANativeWindow*>(native_window);
  return {static_cast<std::uint32_t>(std::max(1, ANativeWindow_getWidth(window))),
          static_cast<std::uint32_t>(std::max(1, ANativeWindow_getHeight(window)))};
#elif defined(_WIN32)
  RECT rect{};
  if (native_window == nullptr || GetClientRect(static_cast<HWND>(native_window), &rect) == 0) return {1280u, 720u};
  return {static_cast<std::uint32_t>(std::max<LONG>(1, rect.right - rect.left)),
          static_cast<std::uint32_t>(std::max<LONG>(1, rect.bottom - rect.top))};
#else
  (void)native_window; return {1280u, 720u};
#endif
}

#if defined(_WIN32)
std::filesystem::path executable_directory() {
  std::array<wchar_t, 32768> path{};
  const DWORD count = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
  if (count == 0 || count >= path.size()) return {};
  return std::filesystem::path{std::wstring_view{path.data(), count}}.parent_path();
}
#endif

std::vector<std::byte> load_asset_bytes(void* asset_manager, std::string_view relative_path) {
#if defined(__ANDROID__)
  auto* manager = static_cast<AAssetManager*>(asset_manager);
  if (manager == nullptr) return {};
  const std::string path{relative_path};
  AAsset* asset = AAssetManager_open(manager, path.c_str(), AASSET_MODE_BUFFER);
  if (asset == nullptr) return {};
  const auto length = AAsset_getLength64(asset);
  if (length <= 0 || static_cast<std::uint64_t>(length) > std::numeric_limits<std::size_t>::max()) {
    AAsset_close(asset); return {};
  }
  std::vector<std::byte> bytes(static_cast<std::size_t>(length));
  const int read = AAsset_read(asset, bytes.data(), bytes.size());
  AAsset_close(asset);
  return read == length ? bytes : std::vector<std::byte>{};
#elif defined(_WIN32)
  (void)asset_manager;
  std::ifstream stream(executable_directory() / std::filesystem::path{relative_path}, std::ios::binary | std::ios::ate);
  if (!stream) return {};
  const auto end = stream.tellg(); if (end <= 0) return {};
  const auto size = static_cast<std::size_t>(end); std::vector<std::byte> bytes(size);
  stream.seekg(0, std::ios::beg); stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
  return stream ? bytes : std::vector<std::byte>{};
#else
  (void)asset_manager; (void)relative_path; return {};
#endif
}

std::vector<std::uint32_t> load_spirv(void* asset_manager, const char* name) {
  const auto bytes = load_asset_bytes(asset_manager, std::string{"shaders/"} + name);
  if (bytes.empty() || (bytes.size() % sizeof(std::uint32_t)) != 0u) return {};
  std::vector<std::uint32_t> words(bytes.size() / sizeof(std::uint32_t));
  std::memcpy(words.data(), bytes.data(), bytes.size());
  return words;
}

bool has_extension(VkPhysicalDevice physical, const char* wanted) {
  std::uint32_t count{};
  if (vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, nullptr) != VK_SUCCESS) return false;
  std::vector<VkExtensionProperties> properties(count);
  if (vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, properties.data()) != VK_SUCCESS) return false;
  return std::any_of(properties.begin(), properties.end(), [wanted](const auto& property) {
    return std::string_view{property.extensionName} == wanted;
  });
}

bool choose_device(VulkanRenderer::Impl& state) {
  std::uint32_t count{};
  if (vkEnumeratePhysicalDevices(state.instance, &count, nullptr) != VK_SUCCESS || count == 0) return false;
  std::vector<VkPhysicalDevice> devices(count);
  if (vkEnumeratePhysicalDevices(state.instance, &count, devices.data()) != VK_SUCCESS) return false;
  for (const auto physical : devices) {
    VkPhysicalDeviceProperties properties{}; vkGetPhysicalDeviceProperties(physical, &properties);
    if (properties.apiVersion < VK_API_VERSION_1_2 || !has_extension(physical, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) continue;
    std::uint32_t queue_count{}; vkGetPhysicalDeviceQueueFamilyProperties(physical, &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queues(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &queue_count, queues.data());
    for (std::uint32_t index = 0; index < queue_count; ++index) {
      VkBool32 present = VK_FALSE;
      if (vkGetPhysicalDeviceSurfaceSupportKHR(physical, index, state.surface, &present) != VK_SUCCESS) continue;
      if ((queues[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u && present == VK_TRUE) {
        state.physical = physical; state.queue_family = index; return true;
      }
    }
  }
  return false;
}

bool create_device(VulkanRenderer::Impl& state) {
  constexpr float priority = 1.0f;
  VkDeviceQueueCreateInfo queue_info{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
  queue_info.queueFamilyIndex = state.queue_family; queue_info.queueCount = 1; queue_info.pQueuePriorities = &priority;
  const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  info.queueCreateInfoCount = 1; info.pQueueCreateInfos = &queue_info;
  info.enabledExtensionCount = 1; info.ppEnabledExtensionNames = extensions;
  if (vkCreateDevice(state.physical, &info, nullptr, &state.device) != VK_SUCCESS) return false;
  vkGetDeviceQueue(state.device, state.queue_family, 0, &state.queue);
  return state.queue != VK_NULL_HANDLE;
}

bool find_memory_type(VkPhysicalDevice physical, std::uint32_t mask, VkMemoryPropertyFlags required, std::uint32_t& type_index) {
  VkPhysicalDeviceMemoryProperties properties{}; vkGetPhysicalDeviceMemoryProperties(physical, &properties);
  for (std::uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
    if ((mask & (1u << i)) != 0u && (properties.memoryTypes[i].propertyFlags & required) == required) {
      type_index = i; return true;
    }
  }
  return false;
}

VkCompositeAlphaFlagBitsKHR choose_composite_alpha(VkCompositeAlphaFlagsKHR supported) {
  constexpr VkCompositeAlphaFlagBitsKHR candidates[] = {
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR, VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR};
  for (const auto value : candidates) if ((supported & value) != 0u) return value;
  return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

bool create_swapchain(VulkanRenderer::Impl& state, void* native_window) {
  VkSurfaceCapabilitiesKHR capabilities{};
  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state.physical, state.surface, &capabilities) != VK_SUCCESS) return false;
  state.surface_rotation = surface_rotation_code(capabilities.currentTransform);
  state.window_extent = native_window_extent(native_window);

  std::uint32_t format_count{};
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(state.physical, state.surface, &format_count, nullptr) != VK_SUCCESS || format_count == 0) return false;
  std::vector<VkSurfaceFormatKHR> formats(format_count);
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(state.physical, state.surface, &format_count, formats.data()) != VK_SUCCESS) return false;
  VkSurfaceFormatKHR chosen = formats.front();
  for (const auto& format : formats) {
    if ((format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB) &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) { chosen = format; break; }
  }
  state.format = chosen.format;

  if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
    state.swapchain_extent = capabilities.currentExtent;
  } else {
    state.swapchain_extent.width = std::clamp(state.window_extent.width,
                                              capabilities.minImageExtent.width,
                                              capabilities.maxImageExtent.width);
    state.swapchain_extent.height = std::clamp(state.window_extent.height,
                                               capabilities.minImageExtent.height,
                                               capabilities.maxImageExtent.height);
  }

  // The swapchain must match the surface extent. Camera projection is a separate logical
  // coordinate system: a 90/270 degree pre-rotation swaps logical width/height, but must not
  // silently resize the presentable images.
  state.logical_extent = logical_extent_for_surface_rotation(
      {state.swapchain_extent.width, state.swapchain_extent.height}, state.surface_rotation);

  std::uint32_t image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0) image_count = std::min(image_count, capabilities.maxImageCount);
  VkSwapchainCreateInfoKHR info{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  info.surface = state.surface; info.minImageCount = image_count; info.imageFormat = state.format;
  info.imageColorSpace = chosen.colorSpace; info.imageExtent = state.swapchain_extent; info.imageArrayLayers = 1;
  info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  info.preTransform = capabilities.currentTransform; info.compositeAlpha = choose_composite_alpha(capabilities.supportedCompositeAlpha);
  info.presentMode = VK_PRESENT_MODE_FIFO_KHR; info.clipped = VK_TRUE;
  if (vkCreateSwapchainKHR(state.device, &info, nullptr, &state.swapchain) != VK_SUCCESS) return false;
  if (vkGetSwapchainImagesKHR(state.device, state.swapchain, &image_count, nullptr) != VK_SUCCESS || image_count == 0) return false;
  state.images.resize(image_count);
  if (vkGetSwapchainImagesKHR(state.device, state.swapchain, &image_count, state.images.data()) != VK_SUCCESS) return false;
  state.estimated_bytes = static_cast<std::uint64_t>(state.swapchain_extent.width) *
                          state.swapchain_extent.height * 4ull * state.images.size();
  return true;
}

VkFormat choose_depth_format(VkPhysicalDevice physical) {
  for (const auto format : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM}) {
    VkFormatProperties properties{}; vkGetPhysicalDeviceFormatProperties(physical, format, &properties);
    if ((properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0u) return format;
  }
  return VK_FORMAT_UNDEFINED;
}

bool create_depth(VulkanRenderer::Impl& state) {
  state.depth_format = choose_depth_format(state.physical); if (state.depth_format == VK_FORMAT_UNDEFINED) return false;
  VkImageCreateInfo image{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
  image.imageType = VK_IMAGE_TYPE_2D; image.format = state.depth_format;
  image.extent = {state.swapchain_extent.width, state.swapchain_extent.height, 1u}; image.mipLevels = 1; image.arrayLayers = 1;
  image.samples = VK_SAMPLE_COUNT_1_BIT; image.tiling = VK_IMAGE_TILING_OPTIMAL;
  image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; image.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  if (vkCreateImage(state.device, &image, nullptr, &state.depth_image) != VK_SUCCESS) return false;
  VkMemoryRequirements requirements{}; vkGetImageMemoryRequirements(state.device, state.depth_image, &requirements);
  std::uint32_t type{}; if (!find_memory_type(state.physical, requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, type)) return false;
  VkMemoryAllocateInfo allocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocation.allocationSize = requirements.size; allocation.memoryTypeIndex = type;
  if (vkAllocateMemory(state.device, &allocation, nullptr, &state.depth_memory) != VK_SUCCESS ||
      vkBindImageMemory(state.device, state.depth_image, state.depth_memory, 0) != VK_SUCCESS) return false;
  VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
  view.image = state.depth_image; view.viewType = VK_IMAGE_VIEW_TYPE_2D; view.format = state.depth_format;
  view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT; view.subresourceRange.levelCount = 1; view.subresourceRange.layerCount = 1;
  if (vkCreateImageView(state.device, &view, nullptr, &state.depth_view) != VK_SUCCESS) return false;
  state.estimated_bytes += static_cast<std::uint64_t>(state.swapchain_extent.width) * state.swapchain_extent.height * 4ull;
  return true;
}

VkFormat choose_shadow_depth_format(VkPhysicalDevice physical) {
  constexpr VkFormatFeatureFlags required =
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
      VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
  for (const auto format : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D16_UNORM}) {
    VkFormatProperties properties{};
    vkGetPhysicalDeviceFormatProperties(physical, format, &properties);
    if ((properties.optimalTilingFeatures & required) == required) return format;
  }
  return VK_FORMAT_UNDEFINED;
}

bool create_shadow_resources(VulkanRenderer::Impl& state) {
  state.shadow_depth_format = choose_shadow_depth_format(state.physical);
  if (state.shadow_depth_format == VK_FORMAT_UNDEFINED) return false;

  auto create_depth_target = [&](VkImage& image, VkDeviceMemory& memory, VkImageView& view) {
    VkImageCreateInfo info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    info.imageType=VK_IMAGE_TYPE_2D; info.format=state.shadow_depth_format;
    info.extent={kShadowMapSize,kShadowMapSize,1u}; info.mipLevels=1; info.arrayLayers=1;
    info.samples=VK_SAMPLE_COUNT_1_BIT; info.tiling=VK_IMAGE_TILING_OPTIMAL;
    info.usage=VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT|VK_IMAGE_USAGE_SAMPLED_BIT;
    info.sharingMode=VK_SHARING_MODE_EXCLUSIVE; info.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    if(vkCreateImage(state.device,&info,nullptr,&image)!=VK_SUCCESS) return false;
    VkMemoryRequirements req{}; vkGetImageMemoryRequirements(state.device,image,&req);
    std::uint32_t type{};
    if(!find_memory_type(state.physical,req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,type)) return false;
    VkMemoryAllocateInfo alloc{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    alloc.allocationSize=req.size; alloc.memoryTypeIndex=type;
    if(vkAllocateMemory(state.device,&alloc,nullptr,&memory)!=VK_SUCCESS ||
       vkBindImageMemory(state.device,image,memory,0)!=VK_SUCCESS) return false;
    state.estimated_bytes += req.size;
    VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    vi.image=image; vi.viewType=VK_IMAGE_VIEW_TYPE_2D; vi.format=state.shadow_depth_format;
    vi.subresourceRange.aspectMask=VK_IMAGE_ASPECT_DEPTH_BIT;
    vi.subresourceRange.levelCount=1; vi.subresourceRange.layerCount=1;
    return vkCreateImageView(state.device,&vi,nullptr,&view)==VK_SUCCESS;
  };

  if(!create_depth_target(state.shadow_depth_image,state.shadow_depth_memory,state.shadow_depth_view) ||
     !create_depth_target(state.self_shadow_depth_image,state.self_shadow_depth_memory,state.self_shadow_depth_view)) return false;

  VkSamplerCreateInfo sampler{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
  sampler.magFilter=VK_FILTER_NEAREST; sampler.minFilter=VK_FILTER_NEAREST;
  sampler.mipmapMode=VK_SAMPLER_MIPMAP_MODE_NEAREST;
  sampler.addressModeU=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler.addressModeV=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler.addressModeW=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  sampler.borderColor=VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  if(vkCreateSampler(state.device,&sampler,nullptr,&state.shadow_sampler)!=VK_SUCCESS) return false;

  VkAttachmentDescription a{}; a.format=state.shadow_depth_format; a.samples=VK_SAMPLE_COUNT_1_BIT;
  a.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR; a.storeOp=VK_ATTACHMENT_STORE_OP_STORE;
  a.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE; a.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
  a.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED; a.finalLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  VkAttachmentReference dr{0u,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
  VkSubpassDescription sub{}; sub.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS; sub.pDepthStencilAttachment=&dr;
  VkSubpassDependency dep{}; dep.srcSubpass=0; dep.dstSubpass=VK_SUBPASS_EXTERNAL;
  dep.srcStageMask=VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT; dep.dstStageMask=VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  dep.srcAccessMask=VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT; dep.dstAccessMask=VK_ACCESS_SHADER_READ_BIT;
  VkRenderPassCreateInfo pi{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  pi.attachmentCount=1; pi.pAttachments=&a; pi.subpassCount=1; pi.pSubpasses=&sub; pi.dependencyCount=1; pi.pDependencies=&dep;
  if(vkCreateRenderPass(state.device,&pi,nullptr,&state.shadow_render_pass)!=VK_SUCCESS) return false;

  auto make_fb=[&](VkImageView view,VkFramebuffer& fb){
    VkFramebufferCreateInfo fi{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fi.renderPass=state.shadow_render_pass; fi.attachmentCount=1; fi.pAttachments=&view;
    fi.width=kShadowMapSize; fi.height=kShadowMapSize; fi.layers=1;
    return vkCreateFramebuffer(state.device,&fi,nullptr,&fb)==VK_SUCCESS;
  };
  return make_fb(state.shadow_depth_view,state.shadow_framebuffer) &&
         make_fb(state.self_shadow_depth_view,state.self_shadow_framebuffer);
}

bool create_render_targets(VulkanRenderer::Impl& state) {
  state.views.resize(state.images.size());
  for (std::size_t i = 0; i < state.images.size(); ++i) {
    VkImageViewCreateInfo view{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view.image = state.images[i]; view.viewType = VK_IMAGE_VIEW_TYPE_2D; view.format = state.format;
    view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; view.subresourceRange.levelCount = 1; view.subresourceRange.layerCount = 1;
    if (vkCreateImageView(state.device, &view, nullptr, &state.views[i]) != VK_SUCCESS) return false;
  }
  if (!create_depth(state)) return false;
  VkAttachmentDescription attachments[2]{};
  attachments[0].format = state.format; attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachments[1].format = state.depth_format; attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  VkAttachmentReference color{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  VkAttachmentReference depth{1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
  VkSubpassDescription subpass{}; subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1; subpass.pColorAttachments = &color; subpass.pDepthStencilAttachment = &depth;
  VkSubpassDependency dependency{}; dependency.srcSubpass = VK_SUBPASS_EXTERNAL; dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstStageMask = dependency.srcStageMask;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  VkRenderPassCreateInfo pass{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  pass.attachmentCount = 2; pass.pAttachments = attachments; pass.subpassCount = 1; pass.pSubpasses = &subpass;
  pass.dependencyCount = 1; pass.pDependencies = &dependency;
  if (vkCreateRenderPass(state.device, &pass, nullptr, &state.render_pass) != VK_SUCCESS) return false;
  state.framebuffers.resize(state.views.size());
  for (std::size_t i = 0; i < state.views.size(); ++i) {
    const VkImageView framebuffer_attachments[] = {state.views[i], state.depth_view};
    VkFramebufferCreateInfo framebuffer{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebuffer.renderPass = state.render_pass; framebuffer.attachmentCount = 2;
    framebuffer.pAttachments = framebuffer_attachments; framebuffer.width = state.swapchain_extent.width;
    framebuffer.height = state.swapchain_extent.height; framebuffer.layers = 1;
    if (vkCreateFramebuffer(state.device, &framebuffer, nullptr, &state.framebuffers[i]) != VK_SUCCESS) return false;
  }
  VkCommandPoolCreateInfo pool{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  pool.queueFamilyIndex = state.queue_family; pool.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  if (vkCreateCommandPool(state.device, &pool, nullptr, &state.command_pool) != VK_SUCCESS) return false;
  state.command_buffers.resize(state.images.size());
  VkCommandBufferAllocateInfo allocation{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocation.commandPool = state.command_pool; allocation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocation.commandBufferCount = static_cast<std::uint32_t>(state.command_buffers.size());
  return vkAllocateCommandBuffers(state.device, &allocation, state.command_buffers.data()) == VK_SUCCESS;
}

bool create_host_buffer(VulkanRenderer::Impl& state, const void* source, VkDeviceSize size,
                        VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& memory) {
  VkBufferCreateInfo info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  info.size = size; info.usage = usage; info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (vkCreateBuffer(state.device, &info, nullptr, &buffer) != VK_SUCCESS) return false;
  VkMemoryRequirements requirements{}; vkGetBufferMemoryRequirements(state.device, buffer, &requirements);
  std::uint32_t type{};
  if (!find_memory_type(state.physical, requirements.memoryTypeBits,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, type)) return false;
  VkMemoryAllocateInfo allocation{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocation.allocationSize = requirements.size; allocation.memoryTypeIndex = type;
  if (vkAllocateMemory(state.device, &allocation, nullptr, &memory) != VK_SUCCESS ||
      vkBindBufferMemory(state.device, buffer, memory, 0) != VK_SUCCESS) return false;
  void* mapped = nullptr;
  if (vkMapMemory(state.device, memory, 0, size, 0, &mapped) != VK_SUCCESS) return false;
  std::memcpy(mapped, source, static_cast<std::size_t>(size)); vkUnmapMemory(state.device, memory);
  state.estimated_bytes += static_cast<std::uint64_t>(size);
  return true;
}

bool create_frame_lighting_resources(VulkanRenderer::Impl& state) {
  VkDescriptorSetLayoutBinding bindings[3]{};
  bindings[0].binding=0; bindings[0].descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  bindings[0].descriptorCount=1; bindings[0].stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT;
  for(std::uint32_t i=1;i<=2;++i){
    bindings[i].binding=i; bindings[i].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[i].descriptorCount=1; bindings[i].stageFlags=VK_SHADER_STAGE_FRAGMENT_BIT;
  }
  VkDescriptorSetLayoutCreateInfo li{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
  li.bindingCount=3; li.pBindings=bindings;
  if(vkCreateDescriptorSetLayout(state.device,&li,nullptr,&state.frame_lighting_set_layout)!=VK_SUCCESS) return false;

  VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bi.size=sizeof(FrameLightingGpu); bi.usage=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; bi.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
  if(vkCreateBuffer(state.device,&bi,nullptr,&state.frame_lighting_buffer)!=VK_SUCCESS) return false;
  VkMemoryRequirements req{}; vkGetBufferMemoryRequirements(state.device,state.frame_lighting_buffer,&req);
  std::uint32_t type{};
  if(!find_memory_type(state.physical,req.memoryTypeBits,VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,type)) return false;
  VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO}; ai.allocationSize=req.size; ai.memoryTypeIndex=type;
  if(vkAllocateMemory(state.device,&ai,nullptr,&state.frame_lighting_memory)!=VK_SUCCESS ||
     vkBindBufferMemory(state.device,state.frame_lighting_buffer,state.frame_lighting_memory,0)!=VK_SUCCESS) return false;
  if(vkMapMemory(state.device,state.frame_lighting_memory,0,sizeof(FrameLightingGpu),0,&state.frame_lighting_mapped)!=VK_SUCCESS) return false;

  VkDescriptorPoolSize ps[2]{};
  ps[0].type=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; ps[0].descriptorCount=1;
  ps[1].type=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; ps[1].descriptorCount=2;
  VkDescriptorPoolCreateInfo pci{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  pci.maxSets=1; pci.poolSizeCount=2; pci.pPoolSizes=ps;
  if(vkCreateDescriptorPool(state.device,&pci,nullptr,&state.frame_lighting_descriptor_pool)!=VK_SUCCESS) return false;
  VkDescriptorSetAllocateInfo si{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  si.descriptorPool=state.frame_lighting_descriptor_pool; si.descriptorSetCount=1; si.pSetLayouts=&state.frame_lighting_set_layout;
  if(vkAllocateDescriptorSets(state.device,&si,&state.frame_lighting_descriptor_set)!=VK_SUCCESS) return false;

  VkDescriptorBufferInfo db{}; db.buffer=state.frame_lighting_buffer; db.range=sizeof(FrameLightingGpu);
  VkDescriptorImageInfo coarse{}; coarse.sampler=state.shadow_sampler; coarse.imageView=state.shadow_depth_view;
  coarse.imageLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  VkDescriptorImageInfo focused{}; focused.sampler=state.shadow_sampler; focused.imageView=state.self_shadow_depth_view;
  focused.imageLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
  VkWriteDescriptorSet writes[3]{};
  writes[0]={VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET}; writes[0].dstSet=state.frame_lighting_descriptor_set;
  writes[0].dstBinding=0; writes[0].descriptorCount=1; writes[0].descriptorType=VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; writes[0].pBufferInfo=&db;
  writes[1]={VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET}; writes[1].dstSet=state.frame_lighting_descriptor_set;
  writes[1].dstBinding=1; writes[1].descriptorCount=1; writes[1].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; writes[1].pImageInfo=&coarse;
  writes[2]={VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET}; writes[2].dstSet=state.frame_lighting_descriptor_set;
  writes[2].dstBinding=2; writes[2].descriptorCount=1; writes[2].descriptorType=VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; writes[2].pImageInfo=&focused;
  vkUpdateDescriptorSets(state.device,3,writes,0,nullptr);
  const auto initial=make_frame_lighting_gpu(state.lighting);
  std::memcpy(state.frame_lighting_mapped,&initial,sizeof(initial));
  state.estimated_bytes += sizeof(FrameLightingGpu);
  return true;
}

void update_frame_lighting_buffer(VulkanRenderer::Impl& state) noexcept {
  if (state.frame_lighting_mapped == nullptr) return;
  const auto gpu = make_frame_lighting_gpu(state.lighting);
  std::memcpy(state.frame_lighting_mapped, &gpu, sizeof(gpu));
}

bool create_profile_mesh(VulkanRenderer::Impl& state, void* asset_manager,
                         std::string_view path, VulkanRenderer::Impl::ProfileMeshGpu& gpu_mesh) {
  const auto bytes = load_asset_bytes(asset_manager, path);
  if (bytes.empty()) return false;
  RMeshV0 mesh{}; std::string error;
  if (!decode_rmesh(bytes, mesh, error) || mesh.vertices.empty() || mesh.indices.empty()) return false;

  std::vector<GpuVertex> vertices(mesh.vertices.size());
  std::array<float, 3> minimum = mesh.vertices.front().position;
  std::array<float, 3> maximum = mesh.vertices.front().position;
  for (std::size_t i = 0; i < mesh.vertices.size(); ++i) {
    const auto& source = mesh.vertices[i]; auto& target = vertices[i];
    std::copy(source.position.begin(), source.position.end(), target.position);
    std::copy(source.normal.begin(), source.normal.end(), target.normal);
    std::copy(source.uv.begin(), source.uv.end(), target.uv); target.region = source.region_id;
    for (std::size_t component = 0; component < 3; ++component) {
      minimum[component] = std::min(minimum[component], source.position[component]);
      maximum[component] = std::max(maximum[component], source.position[component]);
    }
  }

  const float height = maximum[1] - minimum[1];
  if (!(height > 1.0e-5f)) return false;
  for (std::size_t component = 0; component < 3; ++component) {
    gpu_mesh.mesh_center[component] = 0.5f * (minimum[component] + maximum[component]);
  }
  gpu_mesh.meters_per_unit = 1.75f / height;
  if (mesh.indices.size() > std::numeric_limits<std::uint32_t>::max()) return false;
  gpu_mesh.index_count = static_cast<std::uint32_t>(mesh.indices.size());

  return create_host_buffer(state, vertices.data(), vertices.size() * sizeof(GpuVertex),
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, gpu_mesh.vertex_buffer, gpu_mesh.vertex_memory) &&
         create_host_buffer(state, mesh.indices.data(), mesh.indices.size() * sizeof(std::uint32_t),
                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT, gpu_mesh.index_buffer, gpu_mesh.index_memory);
}

bool create_mesh_buffers(VulkanRenderer::Impl& state, void* asset_manager) {
  constexpr std::array<std::string_view, 2> body_paths{
      "meshes/human_profile_0.rmesh",
      "meshes/human_profile_1.rmesh"};
  constexpr std::array<std::string_view, 2> eye_paths{
      "meshes/eye_profile_0.rmesh",
      "meshes/eye_profile_1.rmesh"};

  for (std::size_t i = 0; i < body_paths.size(); ++i) {
    if (!create_profile_mesh(state, asset_manager, body_paths[i], state.profile_meshes[i])) return false;
    if (!create_profile_mesh(state, asset_manager, eye_paths[i], state.eye_meshes[i])) return false;
  }
  state.camera.set_subject_height(1.75f);
  return true;
}

bool create_pipeline(VulkanRenderer::Impl& state, void* asset_manager) {
  const auto vertex_code = load_spirv(asset_manager, "human.vert.spv");
  const auto fragment_code = load_spirv(asset_manager, "human.frag.spv");
  auto make_module = [&](const std::vector<std::uint32_t>& code, VkShaderModule& module) {
    if (code.empty()) return false;
    VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.codeSize = code.size() * sizeof(std::uint32_t); info.pCode = code.data();
    return vkCreateShaderModule(state.device, &info, nullptr, &module) == VK_SUCCESS;
  };
  VkShaderModule vertex_module = VK_NULL_HANDLE, fragment_module = VK_NULL_HANDLE;
  if (!make_module(vertex_code, vertex_module) || !make_module(fragment_code, fragment_module)) {
    if (vertex_module) vkDestroyShaderModule(state.device, vertex_module, nullptr);
    if (fragment_module) vkDestroyShaderModule(state.device, fragment_module, nullptr);
    return false;
  }
  VkPipelineShaderStageCreateInfo stages[2]{};
  stages[0] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}; stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vertex_module; stages[0].pName = "main";
  stages[1] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}; stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = fragment_module; stages[1].pName = "main";
  VkVertexInputBindingDescription binding{0, sizeof(GpuVertex), VK_VERTEX_INPUT_RATE_VERTEX};
  std::array<VkVertexInputAttributeDescription, 4> attributes{{
      {0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(GpuVertex,position)},
      {1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(GpuVertex,normal)},
      {2,0,VK_FORMAT_R32G32_SFLOAT,offsetof(GpuVertex,uv)},
      {3,0,VK_FORMAT_R32_UINT,offsetof(GpuVertex,region)}}};
  VkPipelineVertexInputStateCreateInfo vertex_input{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vertex_input.vertexBindingDescriptionCount = 1; vertex_input.pVertexBindingDescriptions = &binding;
  vertex_input.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributes.size());
  vertex_input.pVertexAttributeDescriptions = attributes.data();
  VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  VkViewport viewport{0.0f, 0.0f, static_cast<float>(state.swapchain_extent.width),
                      static_cast<float>(state.swapchain_extent.height), 0.0f, 1.0f};
  VkRect2D scissor{}; scissor.extent = state.swapchain_extent;
  VkPipelineViewportStateCreateInfo viewport_state{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewport_state.viewportCount = 1; viewport_state.pViewports = &viewport; viewport_state.scissorCount = 1; viewport_state.pScissors = &scissor;
  VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  raster.polygonMode = VK_POLYGON_MODE_FILL; raster.cullMode = VK_CULL_MODE_NONE;
  raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; raster.lineWidth = 1.0f;
  VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  VkPipelineDepthStencilStateCreateInfo depth{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  depth.depthTestEnable = VK_TRUE; depth.depthWriteEnable = VK_TRUE; depth.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  VkPipelineColorBlendAttachmentState blend_attachment{};
  blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  blend.attachmentCount = 1; blend.pAttachments = &blend_attachment;
  VkPushConstantRange push_range{};
  push_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; push_range.size = sizeof(PushConstants);
  VkPipelineLayoutCreateInfo layout{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  layout.setLayoutCount = 1;
  layout.pSetLayouts = &state.frame_lighting_set_layout;
  layout.pushConstantRangeCount = 1; layout.pPushConstantRanges = &push_range;
  if (vkCreatePipelineLayout(state.device, &layout, nullptr, &state.pipeline_layout) != VK_SUCCESS) {
    vkDestroyShaderModule(state.device, fragment_module, nullptr); vkDestroyShaderModule(state.device, vertex_module, nullptr); return false;
  }
  VkGraphicsPipelineCreateInfo pipeline{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  pipeline.stageCount = 2; pipeline.pStages = stages; pipeline.pVertexInputState = &vertex_input;
  pipeline.pInputAssemblyState = &assembly; pipeline.pViewportState = &viewport_state;
  pipeline.pRasterizationState = &raster; pipeline.pMultisampleState = &multisample;
  pipeline.pDepthStencilState = &depth; pipeline.pColorBlendState = &blend;
  pipeline.layout = state.pipeline_layout; pipeline.renderPass = state.render_pass;
  const auto result = vkCreateGraphicsPipelines(state.device, VK_NULL_HANDLE, 1, &pipeline, nullptr, &state.pipeline);
  vkDestroyShaderModule(state.device, fragment_module, nullptr); vkDestroyShaderModule(state.device, vertex_module, nullptr);
  return result == VK_SUCCESS;
}

bool create_shadow_pipeline(VulkanRenderer::Impl& state, void* asset_manager) {
  const auto vertex_code = load_spirv(asset_manager, "human.vert.spv");
  if (vertex_code.empty()) return false;
  VkShaderModule vertex_module = VK_NULL_HANDLE;
  VkShaderModuleCreateInfo module{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  module.codeSize = vertex_code.size() * sizeof(std::uint32_t);
  module.pCode = vertex_code.data();
  if (vkCreateShaderModule(state.device, &module, nullptr, &vertex_module) != VK_SUCCESS) return false;
  VkPipelineShaderStageCreateInfo stage{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
  stage.module = vertex_module;
  stage.pName = "main";
  VkVertexInputBindingDescription binding{0, sizeof(GpuVertex), VK_VERTEX_INPUT_RATE_VERTEX};
  std::array<VkVertexInputAttributeDescription,4> attrs{{
    {0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(GpuVertex,position)},
    {1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(GpuVertex,normal)},
    {2,0,VK_FORMAT_R32G32_SFLOAT,offsetof(GpuVertex,uv)},
    {3,0,VK_FORMAT_R32_UINT,offsetof(GpuVertex,region)}}};
  VkPipelineVertexInputStateCreateInfo vi{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vi.vertexBindingDescriptionCount=1; vi.pVertexBindingDescriptions=&binding;
  vi.vertexAttributeDescriptionCount=static_cast<std::uint32_t>(attrs.size());
  vi.pVertexAttributeDescriptions=attrs.data();
  VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  ia.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  VkViewport vp{0.0f,0.0f,float(kShadowMapSize),float(kShadowMapSize),0.0f,1.0f};
  VkRect2D sc{}; sc.extent={kShadowMapSize,kShadowMapSize};
  VkPipelineViewportStateCreateInfo vs{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  vs.viewportCount=1; vs.pViewports=&vp; vs.scissorCount=1; vs.pScissors=&sc;
  VkPipelineRasterizationStateCreateInfo rs{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rs.polygonMode=VK_POLYGON_MODE_FILL; rs.cullMode=VK_CULL_MODE_NONE;
  rs.frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE; rs.depthBiasEnable=VK_TRUE;
  rs.depthBiasConstantFactor=1.25f; rs.depthBiasSlopeFactor=1.75f; rs.lineWidth=1.0f;
  VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  ms.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
  VkPipelineDepthStencilStateCreateInfo dp{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  dp.depthTestEnable=VK_TRUE; dp.depthWriteEnable=VK_TRUE; dp.depthCompareOp=VK_COMPARE_OP_LESS_OR_EQUAL;
  VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  VkGraphicsPipelineCreateInfo pi{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  pi.stageCount=1; pi.pStages=&stage; pi.pVertexInputState=&vi; pi.pInputAssemblyState=&ia;
  pi.pViewportState=&vs; pi.pRasterizationState=&rs; pi.pMultisampleState=&ms;
  pi.pDepthStencilState=&dp; pi.pColorBlendState=&cb; pi.layout=state.pipeline_layout;
  pi.renderPass=state.shadow_render_pass;
  auto result=vkCreateGraphicsPipelines(state.device,VK_NULL_HANDLE,1,&pi,nullptr,&state.shadow_pipeline);
  vkDestroyShaderModule(state.device,vertex_module,nullptr);
  return result==VK_SUCCESS;
}

bool create_eye_pipelines(VulkanRenderer::Impl& state, void* asset_manager) {
  const auto vertex_code = load_spirv(asset_manager, "eye.vert.spv");
  const auto fragment_code = load_spirv(asset_manager, "eye.frag.spv");
  auto make_module = [&](const std::vector<std::uint32_t>& code, VkShaderModule& module) {
    if (code.empty()) return false;
    VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.codeSize = code.size() * sizeof(std::uint32_t);
    info.pCode = code.data();
    return vkCreateShaderModule(state.device, &info, nullptr, &module) == VK_SUCCESS;
  };

  VkShaderModule vertex_module = VK_NULL_HANDLE;
  VkShaderModule fragment_module = VK_NULL_HANDLE;
  if (!make_module(vertex_code, vertex_module) || !make_module(fragment_code, fragment_module)) {
    if (vertex_module) vkDestroyShaderModule(state.device, vertex_module, nullptr);
    if (fragment_module) vkDestroyShaderModule(state.device, fragment_module, nullptr);
    return false;
  }

  VkPipelineShaderStageCreateInfo stages[2]{};
  stages[0] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vertex_module;
  stages[0].pName = "main";
  stages[1] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = fragment_module;
  stages[1].pName = "main";

  VkVertexInputBindingDescription binding{0, sizeof(GpuVertex), VK_VERTEX_INPUT_RATE_VERTEX};
  std::array<VkVertexInputAttributeDescription, 4> attributes{{
      {0,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(GpuVertex,position)},
      {1,0,VK_FORMAT_R32G32B32_SFLOAT,offsetof(GpuVertex,normal)},
      {2,0,VK_FORMAT_R32G32_SFLOAT,offsetof(GpuVertex,uv)},
      {3,0,VK_FORMAT_R32_UINT,offsetof(GpuVertex,region)}}};
  VkPipelineVertexInputStateCreateInfo vertex_input{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
  vertex_input.vertexBindingDescriptionCount = 1;
  vertex_input.pVertexBindingDescriptions = &binding;
  vertex_input.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attributes.size());
  vertex_input.pVertexAttributeDescriptions = attributes.data();

  VkPipelineInputAssemblyStateCreateInfo assembly{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
  assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkViewport viewport{0.0f, 0.0f, static_cast<float>(state.swapchain_extent.width),
                      static_cast<float>(state.swapchain_extent.height), 0.0f, 1.0f};
  VkRect2D scissor{};
  scissor.extent = state.swapchain_extent;
  VkPipelineViewportStateCreateInfo viewport_state{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  raster.polygonMode = VK_POLYGON_MODE_FILL;
  raster.cullMode = VK_CULL_MODE_NONE;
  raster.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  raster.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisample{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineDepthStencilStateCreateInfo depth{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  depth.depthTestEnable = VK_TRUE;
  depth.depthWriteEnable = VK_FALSE;
  depth.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

  VkPipelineColorBlendAttachmentState blend_attachment{};
  blend_attachment.blendEnable = VK_TRUE;
  blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
  blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
  blend_attachment.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  VkPipelineColorBlendStateCreateInfo blend{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  blend.attachmentCount = 1;
  blend.pAttachments = &blend_attachment;

  VkPushConstantRange push_range{};
  push_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_range.size = sizeof(EyePushConstants);
  VkPipelineLayoutCreateInfo layout{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  layout.setLayoutCount = 1;
  layout.pSetLayouts = &state.frame_lighting_set_layout;
  layout.pushConstantRangeCount = 1;
  layout.pPushConstantRanges = &push_range;
  if (vkCreatePipelineLayout(state.device, &layout, nullptr, &state.eye_pipeline_layout) != VK_SUCCESS) {
    vkDestroyShaderModule(state.device, fragment_module, nullptr);
    vkDestroyShaderModule(state.device, vertex_module, nullptr);
    return false;
  }

  VkGraphicsPipelineCreateInfo pipeline{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  pipeline.stageCount = 2;
  pipeline.pStages = stages;
  pipeline.pVertexInputState = &vertex_input;
  pipeline.pInputAssemblyState = &assembly;
  pipeline.pViewportState = &viewport_state;
  pipeline.pRasterizationState = &raster;
  pipeline.pMultisampleState = &multisample;
  pipeline.pDepthStencilState = &depth;
  pipeline.pColorBlendState = &blend;
  pipeline.layout = state.eye_pipeline_layout;
  pipeline.renderPass = state.render_pass;

  const auto create_variant = [&](const EyePipelineStateContract contract,
                                  VkPipeline& output) {
    depth.depthWriteEnable = contract.depth_write_enable;
    blend_attachment.blendEnable = contract.blend_enable;
    return vkCreateGraphicsPipelines(
               state.device, VK_NULL_HANDLE, 1, &pipeline, nullptr, &output) == VK_SUCCESS;
  };

  const bool inner_ok =
      create_variant(kInnerEyePipelineState, state.eye_inner_pipeline);
  const bool cornea_ok =
      inner_ok && create_variant(kCorneaEyePipelineState, state.eye_cornea_pipeline);

  vkDestroyShaderModule(state.device, fragment_module, nullptr);
  vkDestroyShaderModule(state.device, vertex_module, nullptr);
  return inner_ok && cornea_ok;
}

bool create_sync(VulkanRenderer::Impl& state) {
  VkSemaphoreCreateInfo semaphore{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  if (vkCreateSemaphore(state.device, &semaphore, nullptr, &state.image_available) != VK_SUCCESS ||
      vkCreateSemaphore(state.device, &semaphore, nullptr, &state.render_finished) != VK_SUCCESS) return false;
  VkFenceCreateInfo fence{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO}; fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  return vkCreateFence(state.device, &fence, nullptr, &state.in_flight) == VK_SUCCESS;
}
} // namespace

VulkanRenderer::VulkanRenderer() : impl_(std::make_unique<Impl>()) {}
VulkanRenderer::~VulkanRenderer() { shutdown(); }
bool VulkanRenderer::ready() const noexcept { return impl_ && impl_->initialized; }
std::uint64_t VulkanRenderer::estimated_gpu_bytes() const noexcept { return impl_ ? impl_->estimated_bytes : 0; }

void VulkanRenderer::orbit_camera(float normalized_dx, float normalized_dy) noexcept {
  if (impl_) impl_->camera.orbit(normalized_dx, normalized_dy);
}
void VulkanRenderer::zoom_camera(float scale) noexcept { if (impl_) impl_->camera.zoom(scale); }
void VulkanRenderer::set_camera_preset(CameraPreset preset) noexcept { if (impl_) impl_->camera.set_preset(preset); }
void VulkanRenderer::reset_camera_view() noexcept { if (impl_) impl_->camera.reset_view(); }
CameraState VulkanRenderer::camera_state() const noexcept { return impl_ ? impl_->camera.state() : CameraState{}; }

void VulkanRenderer::set_diagnostic_mode(DiagnosticRenderMode mode) noexcept {
  if (impl_) impl_->diagnostic_mode = mode;
}
DiagnosticRenderMode VulkanRenderer::diagnostic_mode() const noexcept {
  return impl_ ? impl_->diagnostic_mode : DiagnosticRenderMode::genome_perspective;
}

void VulkanRenderer::set_surface_diagnostic_mode(SurfaceDiagnosticMode mode) noexcept {
  if (impl_) impl_->surface_diagnostic_mode = mode;
}
SurfaceDiagnosticMode VulkanRenderer::surface_diagnostic_mode() const noexcept {
  return impl_ ? impl_->surface_diagnostic_mode : SurfaceDiagnosticMode::none;
}

void VulkanRenderer::set_ui_tooltip_row(int row) noexcept {
  if (!impl_) return;
  impl_->ui_tooltip_row = (row >= 0 && row < 11) ? row : -1;
}

int VulkanRenderer::ui_tooltip_row() const noexcept {
  return impl_ ? impl_->ui_tooltip_row : -1;
}

void VulkanRenderer::set_physiology_preset(PhysiologyPreset preset) noexcept {
  if (impl_) impl_->physiology_preset = preset;
}

PhysiologyPreset VulkanRenderer::physiology_preset() const noexcept {
  return impl_ ? impl_->physiology_preset : PhysiologyPreset::normal;
}

void VulkanRenderer::set_eye_diagnostic_mode(EyeDiagnosticMode mode) noexcept {
  if (impl_) impl_->eye_diagnostic_mode = mode;
}

EyeDiagnosticMode VulkanRenderer::eye_diagnostic_mode() const noexcept {
  return impl_ ? impl_->eye_diagnostic_mode : EyeDiagnosticMode::normal;
}

void VulkanRenderer::set_lighting_preset(LightingPreset preset) noexcept {
  if (!impl_) return;
  impl_->lighting = lighting_for(preset, impl_->lighting.filter);
}

LightingPreset VulkanRenderer::lighting_preset() const noexcept {
  return impl_ ? impl_->lighting.preset : LightingPreset::noon;
}

void VulkanRenderer::set_optical_filter_preset(OpticalFilterPreset preset) noexcept {
  if (!impl_) return;
  impl_->lighting = lighting_for(impl_->lighting.preset, preset);
}

OpticalFilterPreset VulkanRenderer::optical_filter_preset() const noexcept {
  return impl_ ? impl_->lighting.filter : OpticalFilterPreset::clear;
}

LightingRuntimeState VulkanRenderer::lighting_state() const noexcept {
  return impl_ ? impl_->lighting
               : lighting_for(LightingPreset::noon, OpticalFilterPreset::clear);
}

RendererDiagnostics VulkanRenderer::diagnostics() const noexcept {
  RendererDiagnostics out{};
  if (!impl_) return out;
  const auto& state = *impl_;
  const auto camera = state.camera.state();
  out.window_width = state.window_extent.width;
  out.window_height = state.window_extent.height;
  out.swapchain_width = state.swapchain_extent.width;
  out.swapchain_height = state.swapchain_extent.height;
  out.logical_width = state.logical_extent.width;
  out.logical_height = state.logical_extent.height;
  out.surface_rotation = state.surface_rotation;
  out.logical_aspect = extent_aspect(state.logical_extent);
  out.fov_y_radians = camera.fov_y_radians;
  out.camera_distance_m = camera.distance_m;
  out.estimated_gpu_bytes = state.estimated_bytes;
  out.shadow_map_size = kShadowMapSize;
  out.mode = state.diagnostic_mode;
  out.surface_diagnostic = state.surface_diagnostic_mode;
  out.lighting_preset = state.lighting.preset;
  out.optical_filter = state.lighting.filter;
  out.scene_luminance = state.lighting.scene_luminance;
  out.effective_eye_luminance = state.lighting.effective_eye_luminance;
  out.filter_transmission = state.lighting.filter_transmission;
  out.polarization_strength = state.lighting.polarization_strength;
  const auto profile = state.last_profile_index & 1u;
  out.pupil_target_radius = state.eye_runtime[profile].target_pupil_radius;
  out.pupil_current_radius = state.eye_runtime[profile].pupil_radius;
  return out;
}

bool VulkanRenderer::initialize(void* native_window, void* asset_manager) {
  shutdown(); impl_ = std::make_unique<Impl>(); auto& state = *impl_;
  const char* platform_extension = platform_surface_extension(); if (platform_extension == nullptr) return false;
  const char* instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME, platform_extension};
  VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
  app.pApplicationName = "Rengine LSG Prototype"; app.applicationVersion = VK_MAKE_VERSION(0, 7, 0);
  app.pEngineName = "DMC Rengine"; app.engineVersion = VK_MAKE_VERSION(0, 7, 0); app.apiVersion = VK_API_VERSION_1_2;
  VkInstanceCreateInfo instance{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
  instance.pApplicationInfo = &app; instance.enabledExtensionCount = static_cast<std::uint32_t>(std::size(instance_extensions));
  instance.ppEnabledExtensionNames = instance_extensions;
  if (vkCreateInstance(&instance, nullptr, &state.instance) != VK_SUCCESS ||
      !create_platform_surface(state.instance, native_window, state.surface)) { shutdown(); return false; }
  if (!choose_device(state) || !create_device(state) || !create_swapchain(state, native_window) ||
      !create_render_targets(state) || !create_shadow_resources(state) ||
      !create_mesh_buffers(state, asset_manager) ||
      !create_frame_lighting_resources(state) ||
      !create_pipeline(state, asset_manager) || !create_shadow_pipeline(state, asset_manager) ||
      !create_eye_pipelines(state, asset_manager) ||
      !create_sync(state)) { shutdown(); return false; }
  state.initialized = true; return true;
}

bool VulkanRenderer::draw_frame(float time_seconds, std::uint32_t character_index, bool detail_enabled) noexcept {
  if (!ready()) return false;
  auto& state = *impl_;
  if (vkWaitForFences(state.device, 1, &state.in_flight, VK_TRUE, UINT64_MAX) != VK_SUCCESS) return false;
  update_frame_lighting_buffer(state);
  state.last_profile_index = character_index & 1u;
  std::uint32_t image_index{};
  const auto acquire = vkAcquireNextImageKHR(state.device, state.swapchain, UINT64_MAX, state.image_available, VK_NULL_HANDLE, &image_index);
  if (acquire == VK_ERROR_OUT_OF_DATE_KHR || (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR)) return false;
  if (vkResetFences(state.device, 1, &state.in_flight) != VK_SUCCESS) return false;
  const auto command = state.command_buffers[image_index];
  if (vkResetCommandBuffer(command, 0) != VK_SUCCESS) return false;
  VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  if (vkBeginCommandBuffer(command, &begin) != VK_SUCCESS) return false;

  const auto profile_index = character_index & 1u;
  const auto& profile_mesh = state.profile_meshes[profile_index];
  const VkDeviceSize offset = 0;
  const CharacterGenomeV0 genome = builtin_profile(profile_index);
  const DerivedCharacterParameters derived = derive_character_parameters(genome);
  const DerivedEyeParameters derived_eye = derive_eye_parameters(genome);
  const CameraState camera = state.camera.state();
  float eye_dt = state.last_eye_time_seconds > 0.0f ? std::clamp(time_seconds - state.last_eye_time_seconds, 0.0f, 0.25f) : (1.0f / 60.0f);
  state.last_eye_time_seconds = time_seconds;
  update_eye_runtime(state.eye_runtime[profile_index], state.lighting.effective_eye_luminance,
                     derived_eye.pupil_bias, eye_dt);

  PushConstants push{};
  push.center_units[0]=profile_mesh.mesh_center[0]; push.center_units[1]=profile_mesh.mesh_center[1];
  push.center_units[2]=profile_mesh.mesh_center[2]; push.center_units[3]=profile_mesh.meters_per_unit;
  push.camera[0]=camera.yaw_radians; push.camera[1]=camera.pitch_radians;
  push.camera[2]=camera.distance_m; push.camera[3]=camera.target_y_m;
  push.geometry0[0]=derived.height_scale; push.geometry0[1]=derived.shoulder_scale;
  push.geometry0[2]=derived.pelvis_scale; push.geometry0[3]=derived.chest_depth_scale;
  push.geometry1[0]=derived.waist_scale; push.geometry1[1]=derived.muscle_scale;
  push.geometry1[2]=derived.body_fat_scale; push.geometry1[3]=derived.head_scale;
  push.skin0[0]=derived.melanin; push.skin0[1]=derived.haemoglobin;
  push.skin0[2]=derived.oiliness; push.skin0[3]=derived.hydration;
  push.micro0[0]=derived.roughness_bias; push.micro0[1]=derived.pore_density;
  push.micro0[2]=derived.pore_scale; push.micro0[3]=derived.pore_depth;
  push.render[0]=extent_aspect(state.logical_extent); push.render[1]=camera.fov_y_radians;
  push.render[2]=time_seconds; push.render[3]=std::bit_cast<float>(derived.surface_seed_low);
  push.flags[0]=profile_index; push.flags[1]=detail_enabled?1u:0u; push.flags[2]=state.surface_rotation;
  const auto mode_bits=static_cast<std::uint32_t>(state.diagnostic_mode)<<1u;
  const auto camera_bits=static_cast<std::uint32_t>(camera.preset)<<3u;
  const auto tooltip_bits=static_cast<std::uint32_t>(state.ui_tooltip_row>=0?state.ui_tooltip_row:15)<<5u;
  const auto physiology_bits=static_cast<std::uint32_t>(state.physiology_preset)<<9u;
  const auto eye_mode_bits=static_cast<std::uint32_t>(state.eye_diagnostic_mode)<<11u;
  const auto lighting_bits=static_cast<std::uint32_t>(state.lighting.preset)<<13u;
  const auto filter_bits=static_cast<std::uint32_t>(state.lighting.filter)<<15u;
  const auto surface_debug_bits=
      static_cast<std::uint32_t>(state.surface_diagnostic_mode)<<20u;
  push.flags[3]=mode_bits|camera_bits|tooltip_bits|physiology_bits|eye_mode_bits|
                lighting_bits|filter_bits|surface_debug_bits;

  VkClearValue shadow_clear{}; shadow_clear.depthStencil={1.0f,0u};
  VkRenderPassBeginInfo spass{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  spass.renderPass=state.shadow_render_pass; spass.framebuffer=state.shadow_framebuffer;
  spass.renderArea.extent={kShadowMapSize,kShadowMapSize}; spass.clearValueCount=1; spass.pClearValues=&shadow_clear;
  vkCmdBeginRenderPass(command,&spass,VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_GRAPHICS,state.shadow_pipeline);
  vkCmdBindVertexBuffers(command,0,1,&profile_mesh.vertex_buffer,&offset);
  vkCmdBindIndexBuffer(command,profile_mesh.index_buffer,0,VK_INDEX_TYPE_UINT32);
  PushConstants shadow_push=push;
  shadow_push.camera[0]=state.lighting.sun_direction[0];
  shadow_push.camera[1]=state.lighting.sun_direction[1];
  shadow_push.camera[2]=state.lighting.sun_direction[2];
  shadow_push.camera[3]=0.0f;
  shadow_push.flags[3]|=(1u<<17u);
  vkCmdPushConstants(command,state.pipeline_layout,VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(shadow_push),&shadow_push);
  vkCmdDrawIndexed(command,profile_mesh.index_count,1,0,0,0);
  vkCmdEndRenderPass(command);

  VkRenderPassBeginInfo self_spass{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  self_spass.renderPass=state.shadow_render_pass;
  self_spass.framebuffer=state.self_shadow_framebuffer;
  self_spass.renderArea.extent={kShadowMapSize,kShadowMapSize};
  self_spass.clearValueCount=1;
  self_spass.pClearValues=&shadow_clear;
  vkCmdBeginRenderPass(command,&self_spass,VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_GRAPHICS,state.shadow_pipeline);
  vkCmdBindVertexBuffers(command,0,1,&profile_mesh.vertex_buffer,&offset);
  vkCmdBindIndexBuffer(command,profile_mesh.index_buffer,0,VK_INDEX_TYPE_UINT32);
  PushConstants self_shadow_push=shadow_push;
  self_shadow_push.flags[3]|=(1u<<19u);
  vkCmdPushConstants(command,state.pipeline_layout,VK_SHADER_STAGE_VERTEX_BIT,0,sizeof(self_shadow_push),&self_shadow_push);
  vkCmdDrawIndexed(command,profile_mesh.index_count,1,0,0,0);
  vkCmdEndRenderPass(command);

  VkClearValue clears[2]{};
  clears[0].color.float32[0]=0.025f; clears[0].color.float32[1]=0.035f;
  clears[0].color.float32[2]=0.055f; clears[0].color.float32[3]=1.0f;
  clears[1].depthStencil={1.0f,0u};
  VkRenderPassBeginInfo rp{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  rp.renderPass=state.render_pass; rp.framebuffer=state.framebuffers[image_index];
  rp.renderArea.extent=state.swapchain_extent; rp.clearValueCount=2; rp.pClearValues=clears;
  vkCmdBeginRenderPass(command,&rp,VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_GRAPHICS,state.pipeline);
  vkCmdBindDescriptorSets(command,VK_PIPELINE_BIND_POINT_GRAPHICS,state.pipeline_layout,0,1,&state.frame_lighting_descriptor_set,0,nullptr);
  vkCmdBindVertexBuffers(command,0,1,&profile_mesh.vertex_buffer,&offset);
  vkCmdBindIndexBuffer(command,profile_mesh.index_buffer,0,VK_INDEX_TYPE_UINT32);
  PushConstants ground_push=push; ground_push.flags[3]|=(1u<<18u);
  vkCmdPushConstants(command,state.pipeline_layout,VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof(ground_push),&ground_push);
  vkCmdDraw(command,6u,1u,0u,0u);
  vkCmdPushConstants(command,state.pipeline_layout,VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof(push),&push);
  vkCmdDrawIndexed(command,profile_mesh.index_count,1,0,0,0);

  // Pass 4B: nested eye rendering is explicit: opaque inner eye, then transparent cornea.
  const auto& eye_mesh = state.eye_meshes[profile_index];
  vkCmdBindVertexBuffers(command, 0, 1, &eye_mesh.vertex_buffer, &offset);
  vkCmdBindIndexBuffer(command, eye_mesh.index_buffer, 0, VK_INDEX_TYPE_UINT32);

  EyePushConstants eye_push{};
  eye_push.center_units[0] = profile_mesh.mesh_center[0];
  eye_push.center_units[1] = profile_mesh.mesh_center[1];
  eye_push.center_units[2] = profile_mesh.mesh_center[2];
  eye_push.center_units[3] = profile_mesh.meters_per_unit;
  eye_push.camera[0] = camera.yaw_radians;
  eye_push.camera[1] = camera.pitch_radians;
  eye_push.camera[2] = camera.distance_m;
  eye_push.camera[3] = camera.target_y_m;
  eye_push.geometry0[0] = derived.height_scale;
  eye_push.geometry0[1] = derived.shoulder_scale;
  eye_push.geometry0[2] = derived.pelvis_scale;
  eye_push.geometry0[3] = derived.chest_depth_scale;
  eye_push.geometry1[0] = derived.waist_scale;
  eye_push.geometry1[1] = derived.muscle_scale;
  eye_push.geometry1[2] = derived.body_fat_scale;
  eye_push.geometry1[3] = derived.head_scale;
  eye_push.render[0] = extent_aspect(state.logical_extent);
  eye_push.render[1] = camera.fov_y_radians;
  eye_push.render[2] = time_seconds;
  eye_push.eye0[0] = derived_eye.iris_primary[0];
  eye_push.eye0[1] = derived_eye.iris_primary[1];
  eye_push.eye0[2] = derived_eye.iris_primary[2];
  eye_push.eye0[3] = state.eye_runtime[profile_index].pupil_radius;
  eye_push.eye1[0] = derived_eye.iris_secondary[0];
  eye_push.eye1[1] = derived_eye.iris_secondary[1];
  eye_push.eye1[2] = derived_eye.iris_secondary[2];
  eye_push.eye1[3] = derived_eye.sclera_tint;
  eye_push.flags[0] = state.surface_rotation;
  eye_push.flags[1] = profile_index;
  const std::uint32_t eye_common_flags =
      static_cast<std::uint32_t>(genome.eyes.vascularity) |
      (static_cast<std::uint32_t>(state.eye_diagnostic_mode) << 8u);
  eye_push.flags[3] = derived_eye.eye_seed_low;

  const auto draw_eye_pass = [&](EyeRenderPass render_pass, VkPipeline pipeline) {
    eye_push.flags[2] =
        eye_common_flags | (static_cast<std::uint32_t>(render_pass) << 10u);
    vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            state.eye_pipeline_layout, 0, 1,
                            &state.frame_lighting_descriptor_set, 0, nullptr);
    vkCmdPushConstants(command, state.eye_pipeline_layout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(eye_push), &eye_push);
    vkCmdDrawIndexed(command, eye_mesh.index_count, 1, 0, 0, 0);
  };

  if (state.eye_diagnostic_mode != EyeDiagnosticMode::cornea_only) {
    draw_eye_pass(EyeRenderPass::inner, state.eye_inner_pipeline);
  }
  if (state.eye_diagnostic_mode != EyeDiagnosticMode::iris_only) {
    draw_eye_pass(EyeRenderPass::cornea, state.eye_cornea_pipeline);
  }

  vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline);
  vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline_layout,
                          0, 1, &state.frame_lighting_descriptor_set, 0, nullptr);
  push.flags[3] = mode_bits | camera_bits | tooltip_bits | physiology_bits |
                  eye_mode_bits | lighting_bits | filter_bits |
                  surface_debug_bits | 1u;
  vkCmdPushConstants(command, state.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                     0, sizeof(push), &push);
  vkCmdDraw(command, 81u, 1u, 0u, 0u);
  vkCmdEndRenderPass(command);
  if (vkEndCommandBuffer(command) != VK_SUCCESS) return false;

  const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO}; submit.waitSemaphoreCount = 1;
  submit.pWaitSemaphores = &state.image_available; submit.pWaitDstStageMask = &wait_stage;
  submit.commandBufferCount = 1; submit.pCommandBuffers = &command;
  submit.signalSemaphoreCount = 1; submit.pSignalSemaphores = &state.render_finished;
  if (vkQueueSubmit(state.queue, 1, &submit, state.in_flight) != VK_SUCCESS) return false;
  VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  present.waitSemaphoreCount = 1; present.pWaitSemaphores = &state.render_finished;
  present.swapchainCount = 1; present.pSwapchains = &state.swapchain; present.pImageIndices = &image_index;
  const auto result = vkQueuePresentKHR(state.queue, &present);
  return result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR;
}

void VulkanRenderer::shutdown() noexcept {
  if (!impl_) return;
  auto& state = *impl_;
  if (state.device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(state.device);
    if (state.in_flight) vkDestroyFence(state.device, state.in_flight, nullptr);
    if (state.render_finished) vkDestroySemaphore(state.device, state.render_finished, nullptr);
    if (state.image_available) vkDestroySemaphore(state.device, state.image_available, nullptr);
    if (state.command_pool) vkDestroyCommandPool(state.device, state.command_pool, nullptr);
    if (state.shadow_pipeline) vkDestroyPipeline(state.device, state.shadow_pipeline, nullptr);
    if (state.eye_inner_pipeline) vkDestroyPipeline(state.device, state.eye_inner_pipeline, nullptr);
    if (state.eye_cornea_pipeline) vkDestroyPipeline(state.device, state.eye_cornea_pipeline, nullptr);
    if (state.eye_pipeline_layout) vkDestroyPipelineLayout(state.device, state.eye_pipeline_layout, nullptr);
    if (state.frame_lighting_mapped && state.frame_lighting_memory) {
      vkUnmapMemory(state.device, state.frame_lighting_memory);
      state.frame_lighting_mapped = nullptr;
    }
    if (state.frame_lighting_descriptor_pool) {
      vkDestroyDescriptorPool(state.device, state.frame_lighting_descriptor_pool, nullptr);
    }
    if (state.shadow_sampler) vkDestroySampler(state.device, state.shadow_sampler, nullptr);
    if (state.self_shadow_framebuffer) vkDestroyFramebuffer(state.device, state.self_shadow_framebuffer, nullptr);
    if (state.shadow_framebuffer) vkDestroyFramebuffer(state.device, state.shadow_framebuffer, nullptr);
    if (state.shadow_render_pass) vkDestroyRenderPass(state.device, state.shadow_render_pass, nullptr);
    if (state.self_shadow_depth_view) vkDestroyImageView(state.device, state.self_shadow_depth_view, nullptr);
    if (state.self_shadow_depth_image) vkDestroyImage(state.device, state.self_shadow_depth_image, nullptr);
    if (state.self_shadow_depth_memory) vkFreeMemory(state.device, state.self_shadow_depth_memory, nullptr);
    if (state.shadow_depth_view) vkDestroyImageView(state.device, state.shadow_depth_view, nullptr);
    if (state.shadow_depth_image) vkDestroyImage(state.device, state.shadow_depth_image, nullptr);
    if (state.shadow_depth_memory) vkFreeMemory(state.device, state.shadow_depth_memory, nullptr);
    if (state.frame_lighting_set_layout) {
      vkDestroyDescriptorSetLayout(state.device, state.frame_lighting_set_layout, nullptr);
    }
    if (state.frame_lighting_buffer) vkDestroyBuffer(state.device, state.frame_lighting_buffer, nullptr);
    if (state.frame_lighting_memory) vkFreeMemory(state.device, state.frame_lighting_memory, nullptr);
    if (state.pipeline) vkDestroyPipeline(state.device, state.pipeline, nullptr);
    if (state.pipeline_layout) vkDestroyPipelineLayout(state.device, state.pipeline_layout, nullptr);
    for (const auto framebuffer : state.framebuffers) vkDestroyFramebuffer(state.device, framebuffer, nullptr);
    if (state.render_pass) vkDestroyRenderPass(state.device, state.render_pass, nullptr);
    if (state.depth_view) vkDestroyImageView(state.device, state.depth_view, nullptr);
    if (state.depth_image) vkDestroyImage(state.device, state.depth_image, nullptr);
    if (state.depth_memory) vkFreeMemory(state.device, state.depth_memory, nullptr);
    for (const auto view : state.views) vkDestroyImageView(state.device, view, nullptr);
    for (auto& profile_mesh : state.profile_meshes) {
      if (profile_mesh.vertex_buffer) vkDestroyBuffer(state.device, profile_mesh.vertex_buffer, nullptr);
      if (profile_mesh.vertex_memory) vkFreeMemory(state.device, profile_mesh.vertex_memory, nullptr);
      if (profile_mesh.index_buffer) vkDestroyBuffer(state.device, profile_mesh.index_buffer, nullptr);
      if (profile_mesh.index_memory) vkFreeMemory(state.device, profile_mesh.index_memory, nullptr);
    }
    for (auto& eye_mesh : state.eye_meshes) {
      if (eye_mesh.vertex_buffer) vkDestroyBuffer(state.device, eye_mesh.vertex_buffer, nullptr);
      if (eye_mesh.vertex_memory) vkFreeMemory(state.device, eye_mesh.vertex_memory, nullptr);
      if (eye_mesh.index_buffer) vkDestroyBuffer(state.device, eye_mesh.index_buffer, nullptr);
      if (eye_mesh.index_memory) vkFreeMemory(state.device, eye_mesh.index_memory, nullptr);
    }
    if (state.swapchain) vkDestroySwapchainKHR(state.device, state.swapchain, nullptr);
    vkDestroyDevice(state.device, nullptr);
  }
  if (state.surface && state.instance) vkDestroySurfaceKHR(state.instance, state.surface, nullptr);
  if (state.instance) vkDestroyInstance(state.instance, nullptr);
  impl_ = std::make_unique<Impl>();
}

} // namespace rengine::lsg
