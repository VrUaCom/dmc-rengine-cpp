#include "vulkan_renderer.hpp"

#include <vulkan/vulkan.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
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
  std::uint32_t queue_family{0};
  VkSwapchainKHR swapchain{VK_NULL_HANDLE};
  VkFormat format{VK_FORMAT_UNDEFINED};
  VkExtent2D extent{};
  std::vector<VkImage> images;
  std::vector<VkImageView> views;
  VkRenderPass render_pass{VK_NULL_HANDLE};
  std::vector<VkFramebuffer> framebuffers;
  VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
  VkPipeline pipeline{VK_NULL_HANDLE};
  VkCommandPool command_pool{VK_NULL_HANDLE};
  std::vector<VkCommandBuffer> command_buffers;
  VkSemaphore image_available{VK_NULL_HANDLE};
  VkSemaphore render_finished{VK_NULL_HANDLE};
  VkFence in_flight{VK_NULL_HANDLE};
  std::uint64_t estimated_bytes{};
  bool initialized{};
};

namespace {

struct PushConstants {
  std::uint32_t character_index{};
  std::uint32_t detail_enabled{};
  float time_seconds{};
  float reserved{};
};
static_assert(sizeof(PushConstants) == 16);

bool has_extension(VkPhysicalDevice physical, const char* wanted) {
  std::uint32_t count = 0;
  if (vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, nullptr) != VK_SUCCESS) {
    return false;
  }
  std::vector<VkExtensionProperties> properties(count);
  if (vkEnumerateDeviceExtensionProperties(physical, nullptr, &count, properties.data()) != VK_SUCCESS) {
    return false;
  }
  for (const auto& property : properties) {
    if (std::string_view{property.extensionName} == wanted) {
      return true;
    }
  }
  return false;
}

bool choose_device(VulkanRenderer::Impl& state) {
  std::uint32_t count = 0;
  if (vkEnumeratePhysicalDevices(state.instance, &count, nullptr) != VK_SUCCESS || count == 0) {
    return false;
  }
  std::vector<VkPhysicalDevice> devices(count);
  if (vkEnumeratePhysicalDevices(state.instance, &count, devices.data()) != VK_SUCCESS) {
    return false;
  }

  for (const auto physical : devices) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical, &properties);
    if (properties.apiVersion < VK_API_VERSION_1_2 ||
        !has_extension(physical, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
      continue;
    }

    std::uint32_t queue_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &queue_count, nullptr);
    std::vector<VkQueueFamilyProperties> queues(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical, &queue_count, queues.data());

    for (std::uint32_t i = 0; i < queue_count; ++i) {
      VkBool32 present = VK_FALSE;
      if (vkGetPhysicalDeviceSurfaceSupportKHR(physical, i, state.surface, &present) != VK_SUCCESS) {
        continue;
      }
      if ((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u && present == VK_TRUE) {
        state.physical = physical;
        state.queue_family = i;
        return true;
      }
    }
  }
  return false;
}

bool create_device(VulkanRenderer::Impl& state) {
  constexpr float priority = 1.0f;
  VkDeviceQueueCreateInfo queue_info{};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.queueFamilyIndex = state.queue_family;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = &priority;

  const char* extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  VkDeviceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount = 1;
  create_info.pQueueCreateInfos = &queue_info;
  create_info.enabledExtensionCount = 1;
  create_info.ppEnabledExtensionNames = extensions;
  if (vkCreateDevice(state.physical, &create_info, nullptr, &state.device) != VK_SUCCESS) {
    return false;
  }
  vkGetDeviceQueue(state.device, state.queue_family, 0, &state.queue);
  return state.queue != VK_NULL_HANDLE;
}

VkCompositeAlphaFlagBitsKHR choose_composite_alpha(VkCompositeAlphaFlagsKHR supported) {
  constexpr VkCompositeAlphaFlagBitsKHR candidates[] = {
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR};
  for (const auto candidate : candidates) {
    if ((supported & candidate) != 0u) {
      return candidate;
    }
  }
  return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

bool create_swapchain(VulkanRenderer::Impl& state, void* native_window) {
  VkSurfaceCapabilitiesKHR capabilities{};
  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(state.physical, state.surface, &capabilities) != VK_SUCCESS) {
    return false;
  }

  std::uint32_t format_count = 0;
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(state.physical, state.surface, &format_count, nullptr) != VK_SUCCESS ||
      format_count == 0) {
    return false;
  }
  std::vector<VkSurfaceFormatKHR> formats(format_count);
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(state.physical, state.surface, &format_count, formats.data()) != VK_SUCCESS) {
    return false;
  }

  VkSurfaceFormatKHR chosen = formats.front();
  for (const auto& format : formats) {
    if ((format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB) &&
        format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      chosen = format;
      break;
    }
  }
  state.format = chosen.format;

  if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max()) {
    state.extent = capabilities.currentExtent;
  } else {
#if defined(__ANDROID__)
    auto* window = static_cast<ANativeWindow*>(native_window);
    const auto width = static_cast<std::uint32_t>(std::max(1, ANativeWindow_getWidth(window)));
    const auto height = static_cast<std::uint32_t>(std::max(1, ANativeWindow_getHeight(window)));
#else
    const std::uint32_t width = 1280;
    const std::uint32_t height = 720;
#endif
    state.extent.width = std::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    state.extent.height = std::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
  }

  std::uint32_t image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0) {
    image_count = std::min(image_count, capabilities.maxImageCount);
  }

  VkSwapchainCreateInfoKHR create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = state.surface;
  create_info.minImageCount = image_count;
  create_info.imageFormat = state.format;
  create_info.imageColorSpace = chosen.colorSpace;
  create_info.imageExtent = state.extent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  create_info.preTransform = capabilities.currentTransform;
  create_info.compositeAlpha = choose_composite_alpha(capabilities.supportedCompositeAlpha);
  create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
  create_info.clipped = VK_TRUE;
  if (vkCreateSwapchainKHR(state.device, &create_info, nullptr, &state.swapchain) != VK_SUCCESS) {
    return false;
  }

  if (vkGetSwapchainImagesKHR(state.device, state.swapchain, &image_count, nullptr) != VK_SUCCESS || image_count == 0) {
    return false;
  }
  state.images.resize(image_count);
  if (vkGetSwapchainImagesKHR(state.device, state.swapchain, &image_count, state.images.data()) != VK_SUCCESS) {
    return false;
  }
  state.estimated_bytes = static_cast<std::uint64_t>(state.extent.width) *
                          static_cast<std::uint64_t>(state.extent.height) * 4ull *
                          static_cast<std::uint64_t>(state.images.size());
  return true;
}

bool create_render_targets(VulkanRenderer::Impl& state) {
  state.views.resize(state.images.size());
  for (std::size_t i = 0; i < state.images.size(); ++i) {
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = state.images[i];
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = state.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.layerCount = 1;
    if (vkCreateImageView(state.device, &view_info, nullptr, &state.views[i]) != VK_SUCCESS) {
      return false;
    }
  }

  VkAttachmentDescription attachment{};
  attachment.format = state.format;
  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference color_reference{};
  color_reference.attachment = 0;
  color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass{};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_reference;

  VkSubpassDependency dependency{};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass = 0;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  render_pass_info.attachmentCount = 1;
  render_pass_info.pAttachments = &attachment;
  render_pass_info.subpassCount = 1;
  render_pass_info.pSubpasses = &subpass;
  render_pass_info.dependencyCount = 1;
  render_pass_info.pDependencies = &dependency;
  if (vkCreateRenderPass(state.device, &render_pass_info, nullptr, &state.render_pass) != VK_SUCCESS) {
    return false;
  }

  state.framebuffers.resize(state.views.size());
  for (std::size_t i = 0; i < state.views.size(); ++i) {
    VkFramebufferCreateInfo framebuffer_info{};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = state.render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &state.views[i];
    framebuffer_info.width = state.extent.width;
    framebuffer_info.height = state.extent.height;
    framebuffer_info.layers = 1;
    if (vkCreateFramebuffer(state.device, &framebuffer_info, nullptr, &state.framebuffers[i]) != VK_SUCCESS) {
      return false;
    }
  }

  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.queueFamilyIndex = state.queue_family;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  if (vkCreateCommandPool(state.device, &pool_info, nullptr, &state.command_pool) != VK_SUCCESS) {
    return false;
  }

  state.command_buffers.resize(state.images.size());
  VkCommandBufferAllocateInfo allocate_info{};
  allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocate_info.commandPool = state.command_pool;
  allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocate_info.commandBufferCount = static_cast<std::uint32_t>(state.command_buffers.size());
  return vkAllocateCommandBuffers(state.device, &allocate_info, state.command_buffers.data()) == VK_SUCCESS;
}

#if defined(__ANDROID__)
std::vector<std::uint32_t> load_spirv(AAssetManager* manager, const char* path) {
  if (manager == nullptr) {
    return {};
  }
  AAsset* asset = AAssetManager_open(manager, path, AASSET_MODE_BUFFER);
  if (asset == nullptr) {
    return {};
  }
  const auto length = AAsset_getLength64(asset);
  if (length <= 0 || (length % 4) != 0) {
    AAsset_close(asset);
    return {};
  }
  std::vector<std::uint32_t> words(static_cast<std::size_t>(length) / sizeof(std::uint32_t));
  const int read = AAsset_read(asset, words.data(), static_cast<std::size_t>(length));
  AAsset_close(asset);
  if (read != length) {
    return {};
  }
  return words;
}
#endif

bool create_shader_module(VulkanRenderer::Impl& state,
                          const std::vector<std::uint32_t>& code,
                          VkShaderModule& module) {
  if (code.empty()) {
    return false;
  }
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = code.size() * sizeof(std::uint32_t);
  create_info.pCode = code.data();
  return vkCreateShaderModule(state.device, &create_info, nullptr, &module) == VK_SUCCESS;
}

bool create_pipeline(VulkanRenderer::Impl& state, void* asset_manager) {
#if defined(__ANDROID__)
  auto* manager = static_cast<AAssetManager*>(asset_manager);
  const auto vertex_code = load_spirv(manager, "shaders/human.vert.spv");
  const auto fragment_code = load_spirv(manager, "shaders/human.frag.spv");
#else
  (void)asset_manager;
  const std::vector<std::uint32_t> vertex_code;
  const std::vector<std::uint32_t> fragment_code;
#endif

  VkShaderModule vertex_module = VK_NULL_HANDLE;
  VkShaderModule fragment_module = VK_NULL_HANDLE;
  if (!create_shader_module(state, vertex_code, vertex_module) ||
      !create_shader_module(state, fragment_code, fragment_module)) {
    if (vertex_module != VK_NULL_HANDLE) {
      vkDestroyShaderModule(state.device, vertex_module, nullptr);
    }
    if (fragment_module != VK_NULL_HANDLE) {
      vkDestroyShaderModule(state.device, fragment_module, nullptr);
    }
    return false;
  }

  VkPipelineShaderStageCreateInfo stages[2]{};
  stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  stages[0].module = vertex_module;
  stages[0].pName = "main";
  stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  stages[1].module = fragment_module;
  stages[1].pName = "main";

  VkPipelineVertexInputStateCreateInfo vertex_input{};
  vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  VkPipelineInputAssemblyStateCreateInfo input_assembly{};
  input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(state.extent.width);
  viewport.height = static_cast<float>(state.extent.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{};
  scissor.extent = state.extent;

  VkPipelineViewportStateCreateInfo viewport_state{};
  viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewport_state.viewportCount = 1;
  viewport_state.pViewports = &viewport;
  viewport_state.scissorCount = 1;
  viewport_state.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterization{};
  rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterization.polygonMode = VK_POLYGON_MODE_FILL;
  rasterization.cullMode = VK_CULL_MODE_NONE;
  rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterization.lineWidth = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisample{};
  multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState blend_attachment{};
  blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                     VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  VkPipelineColorBlendStateCreateInfo blend{};
  blend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  blend.attachmentCount = 1;
  blend.pAttachments = &blend_attachment;

  VkPushConstantRange push_range{};
  push_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  push_range.size = sizeof(PushConstants);
  VkPipelineLayoutCreateInfo layout_info{};
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.pushConstantRangeCount = 1;
  layout_info.pPushConstantRanges = &push_range;
  if (vkCreatePipelineLayout(state.device, &layout_info, nullptr, &state.pipeline_layout) != VK_SUCCESS) {
    vkDestroyShaderModule(state.device, fragment_module, nullptr);
    vkDestroyShaderModule(state.device, vertex_module, nullptr);
    return false;
  }

  VkGraphicsPipelineCreateInfo pipeline_info{};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = stages;
  pipeline_info.pVertexInputState = &vertex_input;
  pipeline_info.pInputAssemblyState = &input_assembly;
  pipeline_info.pViewportState = &viewport_state;
  pipeline_info.pRasterizationState = &rasterization;
  pipeline_info.pMultisampleState = &multisample;
  pipeline_info.pColorBlendState = &blend;
  pipeline_info.layout = state.pipeline_layout;
  pipeline_info.renderPass = state.render_pass;
  pipeline_info.subpass = 0;
  const VkResult result = vkCreateGraphicsPipelines(
      state.device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &state.pipeline);

  vkDestroyShaderModule(state.device, fragment_module, nullptr);
  vkDestroyShaderModule(state.device, vertex_module, nullptr);
  return result == VK_SUCCESS;
}

bool create_sync(VulkanRenderer::Impl& state) {
  VkSemaphoreCreateInfo semaphore_info{};
  semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  if (vkCreateSemaphore(state.device, &semaphore_info, nullptr, &state.image_available) != VK_SUCCESS ||
      vkCreateSemaphore(state.device, &semaphore_info, nullptr, &state.render_finished) != VK_SUCCESS) {
    return false;
  }

  VkFenceCreateInfo fence_info{};
  fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
  return vkCreateFence(state.device, &fence_info, nullptr, &state.in_flight) == VK_SUCCESS;
}

}  // namespace

VulkanRenderer::VulkanRenderer() : impl_(std::make_unique<Impl>()) {}
VulkanRenderer::~VulkanRenderer() { shutdown(); }

bool VulkanRenderer::ready() const noexcept {
  return impl_ && impl_->initialized;
}

std::uint64_t VulkanRenderer::estimated_gpu_bytes() const noexcept {
  return impl_ ? impl_->estimated_bytes : 0;
}

bool VulkanRenderer::initialize(void* native_window, void* asset_manager) {
  shutdown();
  impl_ = std::make_unique<Impl>();
  auto& state = *impl_;

#if defined(__ANDROID__)
  const char* instance_extensions[] = {
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_KHR_ANDROID_SURFACE_EXTENSION_NAME};
#else
  const char* instance_extensions[] = {VK_KHR_SURFACE_EXTENSION_NAME};
#endif

  VkApplicationInfo application_info{};
  application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  application_info.pApplicationName = "Rengine LSG Prototype";
  application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  application_info.pEngineName = "DMC Rengine";
  application_info.engineVersion = VK_MAKE_VERSION(0, 2, 0);
  application_info.apiVersion = VK_API_VERSION_1_2;

  VkInstanceCreateInfo instance_info{};
  instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_info.pApplicationInfo = &application_info;
  instance_info.enabledExtensionCount = static_cast<std::uint32_t>(std::size(instance_extensions));
  instance_info.ppEnabledExtensionNames = instance_extensions;
  if (vkCreateInstance(&instance_info, nullptr, &state.instance) != VK_SUCCESS) {
    return false;
  }

#if defined(__ANDROID__)
  VkAndroidSurfaceCreateInfoKHR surface_info{};
  surface_info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
  surface_info.window = static_cast<ANativeWindow*>(native_window);
  if (surface_info.window == nullptr ||
      vkCreateAndroidSurfaceKHR(state.instance, &surface_info, nullptr, &state.surface) != VK_SUCCESS) {
    shutdown();
    return false;
  }
#else
  (void)native_window;
  shutdown();
  return false;
#endif

  if (!choose_device(state) || !create_device(state) || !create_swapchain(state, native_window) ||
      !create_render_targets(state) || !create_pipeline(state, asset_manager) || !create_sync(state)) {
    shutdown();
    return false;
  }
  state.initialized = true;
  return true;
}

bool VulkanRenderer::draw_frame(float time_seconds,
                                std::uint32_t character_index,
                                bool detail_enabled) noexcept {
  if (!ready()) {
    return false;
  }
  auto& state = *impl_;
  if (vkWaitForFences(state.device, 1, &state.in_flight, VK_TRUE, UINT64_MAX) != VK_SUCCESS) {
    return false;
  }

  std::uint32_t image_index = 0;
  const VkResult acquire = vkAcquireNextImageKHR(
      state.device, state.swapchain, UINT64_MAX, state.image_available, VK_NULL_HANDLE, &image_index);
  if (acquire == VK_ERROR_OUT_OF_DATE_KHR ||
      (acquire != VK_SUCCESS && acquire != VK_SUBOPTIMAL_KHR)) {
    return false;
  }
  if (vkResetFences(state.device, 1, &state.in_flight) != VK_SUCCESS) {
    return false;
  }

  const auto command_buffer = state.command_buffers[image_index];
  if (vkResetCommandBuffer(command_buffer, 0) != VK_SUCCESS) {
    return false;
  }
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
    return false;
  }

  VkClearValue clear{};
  clear.color.float32[0] = 0.025f;
  clear.color.float32[1] = 0.035f;
  clear.color.float32[2] = 0.055f;
  clear.color.float32[3] = 1.0f;

  VkRenderPassBeginInfo render_info{};
  render_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_info.renderPass = state.render_pass;
  render_info.framebuffer = state.framebuffers[image_index];
  render_info.renderArea.extent = state.extent;
  render_info.clearValueCount = 1;
  render_info.pClearValues = &clear;
  vkCmdBeginRenderPass(command_buffer, &render_info, VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, state.pipeline);
  const PushConstants push{character_index & 1u, detail_enabled ? 1u : 0u, time_seconds, 0.0f};
  vkCmdPushConstants(command_buffer,
                     state.pipeline_layout,
                     VK_SHADER_STAGE_FRAGMENT_BIT,
                     0,
                     sizeof(push),
                     &push);
  vkCmdDraw(command_buffer, 3, 1, 0, 0);
  vkCmdEndRenderPass(command_buffer);
  if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
    return false;
  }

  const VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo submit_info{};
  submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info.waitSemaphoreCount = 1;
  submit_info.pWaitSemaphores = &state.image_available;
  submit_info.pWaitDstStageMask = &wait_stage;
  submit_info.commandBufferCount = 1;
  submit_info.pCommandBuffers = &command_buffer;
  submit_info.signalSemaphoreCount = 1;
  submit_info.pSignalSemaphores = &state.render_finished;
  if (vkQueueSubmit(state.queue, 1, &submit_info, state.in_flight) != VK_SUCCESS) {
    return false;
  }

  VkPresentInfoKHR present_info{};
  present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present_info.waitSemaphoreCount = 1;
  present_info.pWaitSemaphores = &state.render_finished;
  present_info.swapchainCount = 1;
  present_info.pSwapchains = &state.swapchain;
  present_info.pImageIndices = &image_index;
  const VkResult present = vkQueuePresentKHR(state.queue, &present_info);
  return present == VK_SUCCESS || present == VK_SUBOPTIMAL_KHR;
}

void VulkanRenderer::shutdown() noexcept {
  if (!impl_) {
    return;
  }
  auto& state = *impl_;
  if (state.device != VK_NULL_HANDLE) {
    vkDeviceWaitIdle(state.device);
    if (state.in_flight != VK_NULL_HANDLE) {
      vkDestroyFence(state.device, state.in_flight, nullptr);
    }
    if (state.render_finished != VK_NULL_HANDLE) {
      vkDestroySemaphore(state.device, state.render_finished, nullptr);
    }
    if (state.image_available != VK_NULL_HANDLE) {
      vkDestroySemaphore(state.device, state.image_available, nullptr);
    }
    if (state.command_pool != VK_NULL_HANDLE) {
      vkDestroyCommandPool(state.device, state.command_pool, nullptr);
    }
    if (state.pipeline != VK_NULL_HANDLE) {
      vkDestroyPipeline(state.device, state.pipeline, nullptr);
    }
    if (state.pipeline_layout != VK_NULL_HANDLE) {
      vkDestroyPipelineLayout(state.device, state.pipeline_layout, nullptr);
    }
    for (const auto framebuffer : state.framebuffers) {
      vkDestroyFramebuffer(state.device, framebuffer, nullptr);
    }
    if (state.render_pass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(state.device, state.render_pass, nullptr);
    }
    for (const auto view : state.views) {
      vkDestroyImageView(state.device, view, nullptr);
    }
    if (state.swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(state.device, state.swapchain, nullptr);
    }
    vkDestroyDevice(state.device, nullptr);
  }
  if (state.surface != VK_NULL_HANDLE && state.instance != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(state.instance, state.surface, nullptr);
  }
  if (state.instance != VK_NULL_HANDLE) {
    vkDestroyInstance(state.instance, nullptr);
  }
  impl_ = std::make_unique<Impl>();
}

}  // namespace rengine::lsg
