#include "vulkan_renderer.hpp"
#include "rengine/lsg/derived_character.hpp"
#include "rengine/lsg/genome.hpp"
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
  VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
  VkPipeline pipeline{VK_NULL_HANDLE};
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
  CameraController camera{};
  DiagnosticRenderMode diagnostic_mode{DiagnosticRenderMode::genome_perspective};
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
  constexpr std::array<std::string_view, 2> paths{
      "meshes/human_profile_0.rmesh",
      "meshes/human_profile_1.rmesh"};
  for (std::size_t i = 0; i < paths.size(); ++i) {
    if (!create_profile_mesh(state, asset_manager, paths[i], state.profile_meshes[i])) return false;
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
  out.mode = state.diagnostic_mode;
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
      !create_render_targets(state) || !create_mesh_buffers(state, asset_manager) ||
      !create_pipeline(state, asset_manager) || !create_sync(state)) { shutdown(); return false; }
  state.initialized = true; return true;
}

bool VulkanRenderer::draw_frame(float time_seconds, std::uint32_t character_index, bool detail_enabled) noexcept {
  if (!ready()) return false;
  auto& state = *impl_;
  if (vkWaitForFences(state.device, 1, &state.in_flight, VK_TRUE, UINT64_MAX) != VK_SUCCESS) return false;
  std::uint32_t image_index{};
  const auto acquire = vkAcquireNextImageKHR(state.device, state.swapchain, UINT64_MAX, state.image_available, VK_NULL_HANDLE, &image_index);
  if (acquire == VK_ERROR_OUT_OF_DATE_KHR || (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR)) return false;
  if (vkResetFences(state.device, 1, &state.in_flight) != VK_SUCCESS) return false;
  const auto command = state.command_buffers[image_index];
  if (vkResetCommandBuffer(command, 0) != VK_SUCCESS) return false;
  VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  if (vkBeginCommandBuffer(command, &begin) != VK_SUCCESS) return false;
  VkClearValue clears[2]{};
  clears[0].color.float32[0] = 0.025f; clears[0].color.float32[1] = 0.035f;
  clears[0].color.float32[2] = 0.055f; clears[0].color.float32[3] = 1.0f;
  clears[1].depthStencil = {1.0f, 0u};
  VkRenderPassBeginInfo render_pass{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  render_pass.renderPass = state.render_pass; render_pass.framebuffer = state.framebuffers[image_index];
  render_pass.renderArea.extent = state.swapchain_extent; render_pass.clearValueCount = 2; render_pass.pClearValues = clears;
  vkCmdBeginRenderPass(command, &render_pass, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline);
  const auto profile_index = character_index & 1u;
  const auto& profile_mesh = state.profile_meshes[profile_index];
  const VkDeviceSize offset = 0;
  vkCmdBindVertexBuffers(command, 0, 1, &profile_mesh.vertex_buffer, &offset);
  vkCmdBindIndexBuffer(command, profile_mesh.index_buffer, 0, VK_INDEX_TYPE_UINT32);

  const CharacterGenomeV0 genome = builtin_profile(profile_index);
  const DerivedCharacterParameters derived = derive_character_parameters(genome);
  const CameraState camera = state.camera.state();
  PushConstants push{};
  push.center_units[0] = profile_mesh.mesh_center[0]; push.center_units[1] = profile_mesh.mesh_center[1];
  push.center_units[2] = profile_mesh.mesh_center[2]; push.center_units[3] = profile_mesh.meters_per_unit;
  push.camera[0] = camera.yaw_radians; push.camera[1] = camera.pitch_radians;
  push.camera[2] = camera.distance_m; push.camera[3] = camera.target_y_m;
  push.geometry0[0] = derived.height_scale; push.geometry0[1] = derived.shoulder_scale;
  push.geometry0[2] = derived.pelvis_scale; push.geometry0[3] = derived.chest_depth_scale;
  push.geometry1[0] = derived.waist_scale; push.geometry1[1] = derived.muscle_scale;
  push.geometry1[2] = derived.body_fat_scale; push.geometry1[3] = derived.head_scale;
  push.skin0[0] = derived.melanin; push.skin0[1] = derived.haemoglobin;
  push.skin0[2] = derived.oiliness; push.skin0[3] = derived.hydration;
  push.micro0[0] = derived.roughness_bias; push.micro0[1] = derived.pore_density;
  push.micro0[2] = derived.pore_scale; push.micro0[3] = derived.pore_depth;
  push.render[0] = extent_aspect(state.logical_extent);
  push.render[1] = camera.fov_y_radians; push.render[2] = time_seconds;
  push.render[3] = std::bit_cast<float>(derived.surface_seed_low);
  push.flags[0] = profile_index; push.flags[1] = detail_enabled ? 1u : 0u;
  push.flags[2] = state.surface_rotation;
  const auto mode_bits = static_cast<std::uint32_t>(state.diagnostic_mode) << 1u;
  const auto camera_bits = static_cast<std::uint32_t>(camera.preset) << 3u;
  push.flags[3] = mode_bits | camera_bits;
  vkCmdPushConstants(command, state.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                     0, sizeof(push), &push);
  vkCmdDrawIndexed(command, profile_mesh.index_count, 1, 0, 0, 0);

  push.flags[3] = mode_bits | camera_bits | 1u;
  vkCmdPushConstants(command, state.pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                     0, sizeof(push), &push);
  vkCmdDraw(command, 51u, 1u, 0u, 0u);
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
    if (state.swapchain) vkDestroySwapchainKHR(state.device, state.swapchain, nullptr);
    vkDestroyDevice(state.device, nullptr);
  }
  if (state.surface && state.instance) vkDestroySurfaceKHR(state.instance, state.surface, nullptr);
  if (state.instance) vkDestroyInstance(state.instance, nullptr);
  impl_ = std::make_unique<Impl>();
}

} // namespace rengine::lsg
