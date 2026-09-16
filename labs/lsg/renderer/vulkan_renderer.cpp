#include "vulkan_renderer.hpp"
#include <vulkan/vulkan.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <string_view>
#include <vector>

#if defined(__ANDROID__)
#include <android/native_window.h>
#include <vulkan/vulkan_android.h>
#endif

namespace rengine::lsg {
struct VulkanRenderer::Impl {
  VkInstance instance{VK_NULL_HANDLE}; VkSurfaceKHR surface{VK_NULL_HANDLE}; VkPhysicalDevice physical{VK_NULL_HANDLE}; VkDevice device{VK_NULL_HANDLE}; VkQueue queue{VK_NULL_HANDLE}; std::uint32_t queue_family{0}; VkSwapchainKHR swapchain{VK_NULL_HANDLE}; VkFormat format{VK_FORMAT_UNDEFINED}; VkExtent2D extent{}; std::vector<VkImage> images; std::vector<VkImageView> views; VkRenderPass render_pass{VK_NULL_HANDLE}; std::vector<VkFramebuffer> framebuffers; VkCommandPool command_pool{VK_NULL_HANDLE}; std::vector<VkCommandBuffer> command_buffers; VkSemaphore image_available{VK_NULL_HANDLE}; VkSemaphore render_finished{VK_NULL_HANDLE}; VkFence in_flight{VK_NULL_HANDLE}; std::uint64_t estimated_bytes{}; bool initialized{};
};

namespace {
bool has_extension(VkPhysicalDevice physical, const char* wanted) {
  std::uint32_t count=0; if(vkEnumerateDeviceExtensionProperties(physical,nullptr,&count,nullptr)!=VK_SUCCESS) return false; std::vector<VkExtensionProperties> props(count); if(vkEnumerateDeviceExtensionProperties(physical,nullptr,&count,props.data())!=VK_SUCCESS) return false; for(const auto& p:props) if(std::string_view{p.extensionName}==wanted) return true; return false;
}
bool choose_device(VulkanRenderer::Impl& s) {
  std::uint32_t count=0; if(vkEnumeratePhysicalDevices(s.instance,&count,nullptr)!=VK_SUCCESS || count==0) return false; std::vector<VkPhysicalDevice> devices(count); if(vkEnumeratePhysicalDevices(s.instance,&count,devices.data())!=VK_SUCCESS) return false;
  for(auto physical:devices){ VkPhysicalDeviceProperties props{}; vkGetPhysicalDeviceProperties(physical,&props); if(props.apiVersion < VK_API_VERSION_1_2) continue; if(!has_extension(physical,VK_KHR_SWAPCHAIN_EXTENSION_NAME)) continue; std::uint32_t qcount=0; vkGetPhysicalDeviceQueueFamilyProperties(physical,&qcount,nullptr); std::vector<VkQueueFamilyProperties> queues(qcount); vkGetPhysicalDeviceQueueFamilyProperties(physical,&qcount,queues.data()); for(std::uint32_t i=0;i<qcount;++i){ VkBool32 present=VK_FALSE; if(vkGetPhysicalDeviceSurfaceSupportKHR(physical,i,s.surface,&present)!=VK_SUCCESS) continue; if((queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)!=0u && present==VK_TRUE){ s.physical=physical; s.queue_family=i; return true; } } }
  return false;
}
bool create_device(VulkanRenderer::Impl& s){ constexpr float priority=1.0f; VkDeviceQueueCreateInfo q{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO}; q.queueFamilyIndex=s.queue_family; q.queueCount=1; q.pQueuePriorities=&priority; const char* extensions[]={VK_KHR_SWAPCHAIN_EXTENSION_NAME}; VkDeviceCreateInfo ci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO}; ci.queueCreateInfoCount=1; ci.pQueueCreateInfos=&q; ci.enabledExtensionCount=1; ci.ppEnabledExtensionNames=extensions; if(vkCreateDevice(s.physical,&ci,nullptr,&s.device)!=VK_SUCCESS) return false; vkGetDeviceQueue(s.device,s.queue_family,0,&s.queue); return s.queue!=VK_NULL_HANDLE; }
bool create_swapchain(VulkanRenderer::Impl& s, void* native_window){
  VkSurfaceCapabilitiesKHR caps{}; if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s.physical,s.surface,&caps)!=VK_SUCCESS) return false; std::uint32_t fcount=0; vkGetPhysicalDeviceSurfaceFormatsKHR(s.physical,s.surface,&fcount,nullptr); if(fcount==0) return false; std::vector<VkSurfaceFormatKHR> formats(fcount); vkGetPhysicalDeviceSurfaceFormatsKHR(s.physical,s.surface,&fcount,formats.data()); VkSurfaceFormatKHR chosen=formats[0]; for(const auto& f:formats) if((f.format==VK_FORMAT_R8G8B8A8_SRGB || f.format==VK_FORMAT_B8G8R8A8_SRGB) && f.colorSpace==VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){chosen=f;break;} s.format=chosen.format;
  if(caps.currentExtent.width!=std::numeric_limits<std::uint32_t>::max()) s.extent=caps.currentExtent; else {
#if defined(__ANDROID__)
    const auto* window=static_cast<ANativeWindow*>(native_window); const auto w=static_cast<std::uint32_t>(std::max(1,ANativeWindow_getWidth(window))); const auto h=static_cast<std::uint32_t>(std::max(1,ANativeWindow_getHeight(window)));
#else
    const std::uint32_t w=1280,h=720;
#endif
    s.extent.width=std::clamp(w,caps.minImageExtent.width,caps.maxImageExtent.width); s.extent.height=std::clamp(h,caps.minImageExtent.height,caps.maxImageExtent.height);
  }
  std::uint32_t image_count=caps.minImageCount+1; if(caps.maxImageCount>0) image_count=std::min(image_count,caps.maxImageCount); VkSwapchainCreateInfoKHR ci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR}; ci.surface=s.surface; ci.minImageCount=image_count; ci.imageFormat=s.format; ci.imageColorSpace=chosen.colorSpace; ci.imageExtent=s.extent; ci.imageArrayLayers=1; ci.imageUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; ci.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE; ci.preTransform=caps.currentTransform; ci.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; ci.presentMode=VK_PRESENT_MODE_FIFO_KHR; ci.clipped=VK_TRUE; if(vkCreateSwapchainKHR(s.device,&ci,nullptr,&s.swapchain)!=VK_SUCCESS) return false; vkGetSwapchainImagesKHR(s.device,s.swapchain,&image_count,nullptr); s.images.resize(image_count); vkGetSwapchainImagesKHR(s.device,s.swapchain,&image_count,s.images.data()); s.estimated_bytes=static_cast<std::uint64_t>(s.extent.width)*s.extent.height*4ull*s.images.size(); return true;
}
bool create_render_targets(VulkanRenderer::Impl& s){
  s.views.resize(s.images.size()); for(std::size_t i=0;i<s.images.size();++i){ VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO}; vi.image=s.images[i]; vi.viewType=VK_IMAGE_VIEW_TYPE_2D; vi.format=s.format; vi.subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT; vi.subresourceRange.levelCount=1; vi.subresourceRange.layerCount=1; if(vkCreateImageView(s.device,&vi,nullptr,&s.views[i])!=VK_SUCCESS) return false; }
  VkAttachmentDescription attachment{}; attachment.format=s.format; attachment.samples=VK_SAMPLE_COUNT_1_BIT; attachment.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR; attachment.storeOp=VK_ATTACHMENT_STORE_OP_STORE; attachment.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE; attachment.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE; attachment.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED; attachment.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; VkAttachmentReference ref{0,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}; VkSubpassDescription sub{}; sub.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS; sub.colorAttachmentCount=1; sub.pColorAttachments=&ref; VkSubpassDependency dep{}; dep.srcSubpass=VK_SUBPASS_EXTERNAL; dep.dstSubpass=0; dep.srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; dep.dstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; dep.dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; VkRenderPassCreateInfo ri{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO}; ri.attachmentCount=1; ri.pAttachments=&attachment; ri.subpassCount=1; ri.pSubpasses=&sub; ri.dependencyCount=1; ri.pDependencies=&dep; if(vkCreateRenderPass(s.device,&ri,nullptr,&s.render_pass)!=VK_SUCCESS) return false;
  s.framebuffers.resize(s.views.size()); for(std::size_t i=0;i<s.views.size();++i){ VkFramebufferCreateInfo fi{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO}; fi.renderPass=s.render_pass; fi.attachmentCount=1; fi.pAttachments=&s.views[i]; fi.width=s.extent.width; fi.height=s.extent.height; fi.layers=1; if(vkCreateFramebuffer(s.device,&fi,nullptr,&s.framebuffers[i])!=VK_SUCCESS) return false; }
  VkCommandPoolCreateInfo pi{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO}; pi.queueFamilyIndex=s.queue_family; pi.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; if(vkCreateCommandPool(s.device,&pi,nullptr,&s.command_pool)!=VK_SUCCESS) return false; s.command_buffers.resize(s.images.size()); VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO}; ai.commandPool=s.command_pool; ai.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY; ai.commandBufferCount=static_cast<std::uint32_t>(s.command_buffers.size()); return vkAllocateCommandBuffers(s.device,&ai,s.command_buffers.data())==VK_SUCCESS;
}
bool create_sync(VulkanRenderer::Impl& s){ VkSemaphoreCreateInfo si{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO}; if(vkCreateSemaphore(s.device,&si,nullptr,&s.image_available)!=VK_SUCCESS) return false; if(vkCreateSemaphore(s.device,&si,nullptr,&s.render_finished)!=VK_SUCCESS) return false; VkFenceCreateInfo fi{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO}; fi.flags=VK_FENCE_CREATE_SIGNALED_BIT; return vkCreateFence(s.device,&fi,nullptr,&s.in_flight)==VK_SUCCESS; }
}

VulkanRenderer::VulkanRenderer():impl_(std::make_unique<Impl>()){}
VulkanRenderer::~VulkanRenderer(){shutdown();}
bool VulkanRenderer::ready() const noexcept{return impl_ && impl_->initialized;}
std::uint64_t VulkanRenderer::estimated_gpu_bytes() const noexcept{return impl_?impl_->estimated_bytes:0;}

bool VulkanRenderer::initialize(void* native_window){
  shutdown(); impl_=std::make_unique<Impl>(); auto& s=*impl_; const char* instance_extensions[]={VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(__ANDROID__)
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#else
    VK_KHR_SURFACE_EXTENSION_NAME
#endif
  }; VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO}; app.pApplicationName="Rengine LSG Prototype"; app.applicationVersion=VK_MAKE_VERSION(0,1,0); app.pEngineName="DMC Rengine"; app.engineVersion=VK_MAKE_VERSION(0,2,0); app.apiVersion=VK_API_VERSION_1_2; VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO}; ici.pApplicationInfo=&app; ici.enabledExtensionCount=2; ici.ppEnabledExtensionNames=instance_extensions; if(vkCreateInstance(&ici,nullptr,&s.instance)!=VK_SUCCESS) return false;
#if defined(__ANDROID__)
  VkAndroidSurfaceCreateInfoKHR sci{VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR}; sci.window=static_cast<ANativeWindow*>(native_window); if(vkCreateAndroidSurfaceKHR(s.instance,&sci,nullptr,&s.surface)!=VK_SUCCESS){shutdown();return false;}
#else
  (void)native_window; shutdown(); return false;
#endif
  if(!choose_device(s)||!create_device(s)||!create_swapchain(s,native_window)||!create_render_targets(s)||!create_sync(s)){shutdown();return false;} s.initialized=true; return true;
}

bool VulkanRenderer::draw_frame(float time_seconds) noexcept{
  if(!ready()) return false; auto& s=*impl_; if(vkWaitForFences(s.device,1,&s.in_flight,VK_TRUE,UINT64_MAX)!=VK_SUCCESS) return false; vkResetFences(s.device,1,&s.in_flight); std::uint32_t index=0; const VkResult acquire=vkAcquireNextImageKHR(s.device,s.swapchain,UINT64_MAX,s.image_available,VK_NULL_HANDLE,&index); if(acquire==VK_ERROR_OUT_OF_DATE_KHR) return false; if(acquire!=VK_SUCCESS && acquire!=VK_SUBOPTIMAL_KHR) return false;
  auto cmd=s.command_buffers[index]; vkResetCommandBuffer(cmd,0); VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO}; if(vkBeginCommandBuffer(cmd,&bi)!=VK_SUCCESS) return false; const float pulse=0.02f*static_cast<float>((static_cast<int>(time_seconds*2.0f)&1)); VkClearValue clear{}; clear.color.float32[0]=0.055f+pulse; clear.color.float32[1]=0.085f+pulse; clear.color.float32[2]=0.125f+pulse; clear.color.float32[3]=1.0f; VkRenderPassBeginInfo rbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO}; rbi.renderPass=s.render_pass; rbi.framebuffer=s.framebuffers[index]; rbi.renderArea.extent=s.extent; rbi.clearValueCount=1; rbi.pClearValues=&clear; vkCmdBeginRenderPass(cmd,&rbi,VK_SUBPASS_CONTENTS_INLINE); vkCmdEndRenderPass(cmd); if(vkEndCommandBuffer(cmd)!=VK_SUCCESS) return false;
  const VkPipelineStageFlags wait_stage=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO}; submit.waitSemaphoreCount=1; submit.pWaitSemaphores=&s.image_available; submit.pWaitDstStageMask=&wait_stage; submit.commandBufferCount=1; submit.pCommandBuffers=&cmd; submit.signalSemaphoreCount=1; submit.pSignalSemaphores=&s.render_finished; if(vkQueueSubmit(s.queue,1,&submit,s.in_flight)!=VK_SUCCESS) return false; VkPresentInfoKHR present{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR}; present.waitSemaphoreCount=1; present.pWaitSemaphores=&s.render_finished; present.swapchainCount=1; present.pSwapchains=&s.swapchain; present.pImageIndices=&index; const VkResult pr=vkQueuePresentKHR(s.queue,&present); return pr==VK_SUCCESS || pr==VK_SUBOPTIMAL_KHR;
}

void VulkanRenderer::shutdown() noexcept{
  if(!impl_) return; auto& s=*impl_; if(s.device!=VK_NULL_HANDLE) vkDeviceWaitIdle(s.device); if(s.device!=VK_NULL_HANDLE){ if(s.in_flight)vkDestroyFence(s.device,s.in_flight,nullptr); if(s.render_finished)vkDestroySemaphore(s.device,s.render_finished,nullptr); if(s.image_available)vkDestroySemaphore(s.device,s.image_available,nullptr); if(s.command_pool)vkDestroyCommandPool(s.device,s.command_pool,nullptr); for(auto f:s.framebuffers)vkDestroyFramebuffer(s.device,f,nullptr); if(s.render_pass)vkDestroyRenderPass(s.device,s.render_pass,nullptr); for(auto v:s.views)vkDestroyImageView(s.device,v,nullptr); if(s.swapchain)vkDestroySwapchainKHR(s.device,s.swapchain,nullptr); vkDestroyDevice(s.device,nullptr); } if(s.surface && s.instance)vkDestroySurfaceKHR(s.instance,s.surface,nullptr); if(s.instance)vkDestroyInstance(s.instance,nullptr); impl_=std::make_unique<Impl>();
}
}
