#include "rengine/lsg/genome.hpp"
#include "vulkan_renderer.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string_view>

namespace {
struct ViewerState {
  std::uint32_t character_index{};
  bool detail_enabled{true};
};

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == WM_NCCREATE) {
    const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lparam);
    SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
  }
  auto* state = reinterpret_cast<ViewerState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
  switch (message) {
    case WM_KEYDOWN:
      if (wparam == VK_ESCAPE) { DestroyWindow(window); return 0; }
      if (state != nullptr && wparam == '0') { state->character_index = 0; return 0; }
      if (state != nullptr && wparam == '1') { state->character_index = 1; return 0; }
      if (state != nullptr && wparam == 'D') { state->detail_enabled = !state->detail_enabled; return 0; }
      break;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    default: break;
  }
  return DefWindowProcW(window, message, wparam, lparam);
}
}  // namespace

int main(int argc, char** argv) {
  ViewerState state{};
  for (int i = 1; i + 1 < argc; ++i) {
    if (std::string_view{argv[i]} == "--character") state.character_index = static_cast<std::uint32_t>(argv[++i][0] == '1');
  }
  const auto genome = rengine::lsg::encode_genome(rengine::lsg::builtin_profile(state.character_index));
  if (genome.empty()) { std::cerr << "failed to create built-in genome\n"; return 2; }

  const HINSTANCE instance = GetModuleHandleW(nullptr);
  constexpr wchar_t kClassName[] = L"RengineLSGPrototypeWindow";
  WNDCLASSW window_class{}; window_class.lpfnWndProc = window_proc; window_class.hInstance = instance;
  window_class.lpszClassName = kClassName; window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
  if (RegisterClassW(&window_class) == 0) { std::cerr << "RegisterClassW failed\n"; return 3; }
  RECT rectangle{0, 0, 1280, 720};
  constexpr DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  AdjustWindowRect(&rectangle, style, FALSE);
  HWND window = CreateWindowExW(0, kClassName, L"Rengine LSG Prototype - 0/1 profile, D detail",
      style, CW_USEDEFAULT, CW_USEDEFAULT, rectangle.right - rectangle.left, rectangle.bottom - rectangle.top,
      nullptr, nullptr, instance, &state);
  if (window == nullptr) { std::cerr << "CreateWindowExW failed\n"; return 4; }
  ShowWindow(window, SW_SHOWDEFAULT);

  rengine::lsg::VulkanRenderer renderer;
  if (!renderer.initialize(window)) { std::cerr << "Vulkan 1.2 Win32 initialization failed\n"; DestroyWindow(window); return 5; }
  std::cout << "Rengine LSG Windows Vulkan PASS bootstrap; genome=" << genome.size()
            << " bytes; estimated swapchain=" << renderer.estimated_gpu_bytes() << " bytes\n";

  const auto start = std::chrono::steady_clock::now();
  bool running = true;
  while (running) {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
      if (message.message == WM_QUIT) { running = false; break; }
      TranslateMessage(&message); DispatchMessageW(&message);
    }
    if (!running) break;
    const float seconds = std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count();
    if (!renderer.draw_frame(seconds, state.character_index, state.detail_enabled)) {
      std::cerr << "Vulkan frame failed\n"; running = false; break;
    }
    Sleep(1);
  }
  renderer.shutdown();
  return 0;
}
