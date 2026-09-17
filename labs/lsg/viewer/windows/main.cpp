#include "rengine/lsg/camera.hpp"
#include "rengine/lsg/genome.hpp"
#include "vulkan_renderer.hpp"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string_view>

namespace {
struct ViewerState {
  rengine::lsg::VulkanRenderer* renderer{};
  std::uint32_t character_index{};
  bool detail_enabled{true};
  bool dragging{};
  bool hud_candidate{};
  int down_x{};
  int down_y{};
  int last_x{};
  int last_y{};
};

const char* mode_name(rengine::lsg::DiagnosticRenderMode mode) {
  using rengine::lsg::DiagnosticRenderMode;
  switch (mode) {
    case DiagnosticRenderMode::genome_perspective: return "GENOME_PERSPECTIVE";
    case DiagnosticRenderMode::raw_perspective: return "RAW_PERSPECTIVE";
    case DiagnosticRenderMode::raw_orthographic: return "RAW_ORTHOGRAPHIC";
  }
  return "UNKNOWN";
}

void print_diagnostics(const ViewerState& state, const char* reason, float fps = 0.0f, float cpu_ms = 0.0f) {
  if (state.renderer == nullptr) return;
  const auto d = state.renderer->diagnostics();
  std::cout << reason << " mode=" << mode_name(d.mode)
            << " window=" << d.window_width << 'x' << d.window_height
            << " swapchain=" << d.swapchain_width << 'x' << d.swapchain_height
            << " logical=" << d.logical_width << 'x' << d.logical_height
            << " rotation=" << d.surface_rotation
            << " aspect=" << d.logical_aspect
            << " fov=" << d.fov_y_radians
            << " distance=" << d.camera_distance_m
            << "m gpu_est=" << d.estimated_gpu_bytes
            << " fps=" << fps << " cpu_frame_ms=" << cpu_ms << '\n';
}

void cycle_mode(ViewerState& state) {
  if (state.renderer == nullptr) return;
  using rengine::lsg::DiagnosticRenderMode;
  const auto current = state.renderer->diagnostic_mode();
  const auto next = current == DiagnosticRenderMode::genome_perspective ? DiagnosticRenderMode::raw_perspective
                  : current == DiagnosticRenderMode::raw_perspective ? DiagnosticRenderMode::raw_orthographic
                                                                    : DiagnosticRenderMode::genome_perspective;
  state.renderer->set_diagnostic_mode(next);
  print_diagnostics(state, "Diagnostic mode changed");
}

void handle_hud(ViewerState& state, HWND window, int x) {
  RECT rect{}; GetClientRect(window, &rect);
  const int width = static_cast<int>(std::max<LONG>(1, rect.right - rect.left));
  const int quarter = std::max(1, width / 4);
  if (x < quarter) state.character_index = 0;
  else if (x < quarter * 2) state.character_index = 1;
  else if (x < quarter * 3) state.detail_enabled = !state.detail_enabled;
  else cycle_mode(state);
}

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
      if (state != nullptr && wparam == 'M') { cycle_mode(*state); return 0; }
      if (state != nullptr && state->renderer != nullptr && wparam == 'F') { state->renderer->set_camera_preset(rengine::lsg::CameraPreset::full_body); return 0; }
      if (state != nullptr && state->renderer != nullptr && wparam == 'P') { state->renderer->set_camera_preset(rengine::lsg::CameraPreset::portrait); return 0; }
      if (state != nullptr && state->renderer != nullptr && wparam == 'C') { state->renderer->set_camera_preset(rengine::lsg::CameraPreset::extreme_close_up); return 0; }
      break;
    case WM_LBUTTONDOWN:
      if (state != nullptr) {
        RECT rect{}; GetClientRect(window, &rect);
        state->down_x = state->last_x = GET_X_LPARAM(lparam);
        state->down_y = state->last_y = GET_Y_LPARAM(lparam);
        state->hud_candidate = state->down_y >= static_cast<int>(static_cast<float>(rect.bottom - rect.top) * 0.70f);
        state->dragging = !state->hud_candidate;
        SetCapture(window); return 0;
      }
      break;
    case WM_MOUSEMOVE:
      if (state != nullptr && state->renderer != nullptr && (wparam & MK_LBUTTON) != 0u) {
        const int x = GET_X_LPARAM(lparam), y = GET_Y_LPARAM(lparam);
        RECT rect{}; GetClientRect(window, &rect);
        const float width = static_cast<float>(std::max<LONG>(1, rect.right - rect.left));
        const float height = static_cast<float>(std::max<LONG>(1, rect.bottom - rect.top));
        const float movement = std::hypot(static_cast<float>(x - state->down_x), static_cast<float>(y - state->down_y));
        if (state->hud_candidate && movement > 0.025f * std::min(width, height)) state->hud_candidate = false;
        if (!state->hud_candidate) {
          if (!state->dragging) { state->dragging = true; state->last_x = x; state->last_y = y; }
          else {
            state->renderer->orbit_camera(static_cast<float>(x - state->last_x) / width,
                                          -static_cast<float>(y - state->last_y) / height);
            state->last_x = x; state->last_y = y;
          }
        }
        return 0;
      }
      break;
    case WM_LBUTTONUP:
      if (state != nullptr) {
        RECT rect{}; GetClientRect(window, &rect);
        const int x = GET_X_LPARAM(lparam), y = GET_Y_LPARAM(lparam);
        if (state->hud_candidate && y >= static_cast<int>(static_cast<float>(rect.bottom - rect.top) * 0.70f)) handle_hud(*state, window, x);
        state->dragging = false; state->hud_candidate = false; ReleaseCapture(); return 0;
      }
      break;
    case WM_MOUSEWHEEL:
      if (state != nullptr && state->renderer != nullptr) {
        const float notches = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wparam)) / static_cast<float>(WHEEL_DELTA);
        state->renderer->zoom_camera(std::pow(1.12f, notches)); return 0;
      }
      break;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    default: break;
  }
  return DefWindowProcW(window, message, wparam, lparam);
}
} // namespace

int main(int argc, char** argv) {
  rengine::lsg::VulkanRenderer renderer;
  ViewerState state{}; state.renderer = &renderer;
  for (int i = 1; i + 1 < argc; ++i) {
    if (std::string_view{argv[i]} == "--character") state.character_index = static_cast<std::uint32_t>(argv[++i][0] == '1');
  }
  const auto genome = rengine::lsg::encode_genome(rengine::lsg::builtin_profile(state.character_index));
  if (genome.empty()) { std::cerr << "failed to create built-in genome\n"; return 2; }

  const HINSTANCE instance = GetModuleHandleW(nullptr);
  constexpr wchar_t kClassName[] = L"RengineLSGPrototypeWindow";
  WNDCLASSW window_class{}; window_class.lpfnWndProc = window_proc; window_class.hInstance = instance;
  window_class.lpszClassName = kClassName; window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
  if (RegisterClassW(&window_class) == 0) { std::cerr << "RegisterClassW failed\n"; return 3; }
  RECT rectangle{0, 0, 1280, 720};
  constexpr DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  AdjustWindowRect(&rectangle, style, FALSE);
  HWND window = CreateWindowExW(0, kClassName,
      L"Rengine LSG - drag orbit, wheel zoom, F/P/C camera, 0/1 profile, D detail, M diagnostics",
      style, CW_USEDEFAULT, CW_USEDEFAULT, rectangle.right - rectangle.left, rectangle.bottom - rectangle.top,
      nullptr, nullptr, instance, &state);
  if (window == nullptr) { std::cerr << "CreateWindowExW failed\n"; return 4; }
  ShowWindow(window, SW_SHOWDEFAULT);

  if (!renderer.initialize(window)) { std::cerr << "Vulkan 1.2 Win32 initialization failed\n"; DestroyWindow(window); return 5; }
  std::cout << "Rengine LSG Windows interactive viewer PASS bootstrap; genome=" << genome.size()
            << " bytes; estimated GPU bytes=" << renderer.estimated_gpu_bytes() << "\n";
  print_diagnostics(state, "Renderer init");

  const auto start = std::chrono::steady_clock::now();
  auto telemetry_start = start;
  std::uint32_t telemetry_frames = 0;
  bool running = true;
  while (running) {
    MSG message{};
    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0) {
      if (message.message == WM_QUIT) { running = false; break; }
      TranslateMessage(&message); DispatchMessageW(&message);
    }
    if (!running) break;
    const auto now = std::chrono::steady_clock::now();
    const float seconds = std::chrono::duration<float>(now - start).count();
    if (!renderer.draw_frame(seconds, state.character_index, state.detail_enabled)) {
      std::cerr << "Vulkan frame failed\n"; running = false; break;
    }
    ++telemetry_frames;
    const auto after = std::chrono::steady_clock::now();
    const float interval = std::chrono::duration<float>(after - telemetry_start).count();
    if (interval >= 1.0f) {
      const float fps = static_cast<float>(telemetry_frames) / interval;
      const float cpu_ms = interval * 1000.0f / static_cast<float>(telemetry_frames);
      print_diagnostics(state, "Frame telemetry", fps, cpu_ms);
      telemetry_start = after; telemetry_frames = 0;
    }
    Sleep(1);
  }
  renderer.shutdown();
  return 0;
}
