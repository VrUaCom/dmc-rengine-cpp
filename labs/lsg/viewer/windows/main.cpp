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
  bool moved{};
  bool character_menu_open{};
  int pressed_ui_row{-1};
  std::chrono::steady_clock::time_point press_start{};
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
    case DiagnosticRenderMode::genome_joint_debug: return "GENOME_JOINT_DEBUG";
  }
  return "UNKNOWN";
}

const char* surface_diagnostic_name(rengine::lsg::SurfaceDiagnosticMode mode) {
  using rengine::lsg::SurfaceDiagnosticMode;
  switch (mode) {
    case SurfaceDiagnosticMode::none: return "NONE";
    case SurfaceDiagnosticMode::shadow_visibility: return "SHADOW_VISIBILITY";
    case SurfaceDiagnosticMode::shadow_compare: return "SHADOW_COMPARE";
    case SurfaceDiagnosticMode::normals: return "NORMALS";
    case SurfaceDiagnosticMode::regions: return "REGIONS";
  }
  return "UNKNOWN";
}

const char* lighting_name(rengine::lsg::LightingPreset preset) {
  using rengine::lsg::LightingPreset;
  switch (preset) {
    case LightingPreset::morning: return "MORNING";
    case LightingPreset::noon: return "NOON";
    case LightingPreset::evening: return "EVENING";
    case LightingPreset::night: return "NIGHT";
  }
  return "UNKNOWN";
}

const char* filter_name(rengine::lsg::OpticalFilterPreset preset) {
  using rengine::lsg::OpticalFilterPreset;
  switch (preset) {
    case OpticalFilterPreset::clear: return "CLEAR";
    case OpticalFilterPreset::tinted: return "TINTED";
    case OpticalFilterPreset::polarized_approx: return "POLARIZED_APPROX";
  }
  return "UNKNOWN";
}

void print_diagnostics(const ViewerState& state, const char* reason, float fps = 0.0f, float cpu_ms = 0.0f) {
  if (state.renderer == nullptr) return;
  const auto d = state.renderer->diagnostics();
  std::cout << reason << " mode=" << mode_name(d.mode)
            << " surface_diag=" << surface_diagnostic_name(d.surface_diagnostic)
            << " window=" << d.window_width << 'x' << d.window_height
            << " swapchain=" << d.swapchain_width << 'x' << d.swapchain_height
            << " logical=" << d.logical_width << 'x' << d.logical_height
            << " rotation=" << d.surface_rotation
            << " aspect=" << d.logical_aspect
            << " fov=" << d.fov_y_radians
            << " distance=" << d.camera_distance_m
            << "m time=" << lighting_name(d.lighting_preset)
            << " filter=" << filter_name(d.optical_filter)
            << " transmission=" << d.filter_transmission
            << " polar_strength=" << d.polarization_strength
            << " scene_lum=" << d.scene_luminance
            << " eye_lum=" << d.effective_eye_luminance
            << " pupil_target=" << d.pupil_target_radius
            << " pupil=" << d.pupil_current_radius
            << " gpu_est=" << d.estimated_gpu_bytes
            << " fps=" << fps << " cpu_frame_ms=" << cpu_ms << '\n';
}

void cycle_mode(ViewerState& state) {
  if (state.renderer == nullptr) return;
  using rengine::lsg::DiagnosticRenderMode;
  using rengine::lsg::SurfaceDiagnosticMode;
  const auto mode = state.renderer->diagnostic_mode();
  const auto surface = state.renderer->surface_diagnostic_mode();

  if (mode == DiagnosticRenderMode::genome_joint_debug) {
    state.renderer->set_surface_diagnostic_mode(SurfaceDiagnosticMode::none);
    state.renderer->set_diagnostic_mode(DiagnosticRenderMode::genome_perspective);
  } else if (mode == DiagnosticRenderMode::raw_perspective) {
    state.renderer->set_diagnostic_mode(DiagnosticRenderMode::raw_orthographic);
  } else if (mode == DiagnosticRenderMode::raw_orthographic) {
    state.renderer->set_diagnostic_mode(DiagnosticRenderMode::genome_perspective);
  } else if (surface == SurfaceDiagnosticMode::none) {
    state.renderer->set_surface_diagnostic_mode(SurfaceDiagnosticMode::shadow_visibility);
  } else if (surface == SurfaceDiagnosticMode::shadow_visibility) {
    state.renderer->set_surface_diagnostic_mode(SurfaceDiagnosticMode::shadow_compare);
  } else if (surface == SurfaceDiagnosticMode::shadow_compare) {
    state.renderer->set_surface_diagnostic_mode(SurfaceDiagnosticMode::normals);
  } else if (surface == SurfaceDiagnosticMode::normals) {
    state.renderer->set_surface_diagnostic_mode(SurfaceDiagnosticMode::regions);
  } else {
    state.renderer->set_surface_diagnostic_mode(SurfaceDiagnosticMode::none);
    state.renderer->set_diagnostic_mode(DiagnosticRenderMode::raw_perspective);
  }
  print_diagnostics(state, "Diagnostic mode changed");
}

int ui_row_from_point(int x, int y, int width, int height) {
  if (width <= 0 || height <= 0) return -1;
  const float nx = static_cast<float>(x) / static_cast<float>(width);
  const float ny = static_cast<float>(y) / static_cast<float>(height);
  if (nx < 0.020f || nx > 0.185f) return -1;
  for (int row = 0; row < 12; ++row) {
    const float center_y = 0.07f + static_cast<float>(row) * 0.07f;
    if (std::abs(ny - center_y) <= 0.029f) return row;
  }
  return -1;
}

int character_menu_row_from_point(const ViewerState& state, int x, int y, int width, int height) {
  if (width <= 0 || height <= 0) return -1;
  const float nx = static_cast<float>(x) / static_cast<float>(width);
  const float ny = static_cast<float>(y) / static_cast<float>(height);
  if (nx < 0.20f || nx > 0.41f) return -1;
  const int count = state.renderer == nullptr ? 0 : static_cast<int>(std::min<std::uint32_t>(state.renderer->character_profile_count(), 8u));
  for (int row = 0; row < count; ++row) {
    const float center_y = 0.07f + static_cast<float>(row) * 0.07f;
    if (std::abs(ny - center_y) <= 0.030f) return row;
  }
  return -1;
}

void set_character_menu_open(ViewerState& state, bool open) {
  state.character_menu_open = open;
  if (state.renderer != nullptr) {
    state.renderer->set_character_menu_open(open);
    if (open) state.renderer->set_ui_tooltip_row(-1);
  }
}

bool select_character_ordinal(ViewerState& state, std::uint32_t ordinal) {
  if (state.renderer == nullptr) return false;
  std::uint32_t id{};
  if (!state.renderer->character_profile_id_at(ordinal, id)) return false;
  state.character_index = id;
  std::cout << "Character id=" << state.character_index << " / "
            << state.renderer->character_profile_name(id) << " selected\n";
  return true;
}

void update_long_press(ViewerState& state) {
  if (state.renderer == nullptr || state.character_menu_open ||
      !state.hud_candidate || state.moved || state.pressed_ui_row < 0) return;
  if (state.pressed_ui_row != 0 && state.pressed_ui_row != 1) return;
  const auto held = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - state.press_start).count();
  if (held < 600) return;
  set_character_menu_open(state, true);
  std::cout << "Character selector opened\n";
}

bool point_in_ui_panel(int x, int y, int width, int height) {
  if (width <= 0 || height <= 0) return false;
  const float nx = static_cast<float>(x) / static_cast<float>(width);
  const float ny = static_cast<float>(y) / static_cast<float>(height);
  return nx >= 0.010f && nx <= 0.195f && ny >= 0.035f && ny <= 0.890f;
}

void handle_ui_row(ViewerState& state, int row) {
  if (state.renderer == nullptr) return;
  set_character_menu_open(state, false);
  using rengine::lsg::CameraPreset;
  if (row != 11) state.renderer->cancel_shadow_probe();
  switch (row) {
    case 0: select_character_ordinal(state, 0); break;
    case 1: select_character_ordinal(state, 1); break;
    case 2: state.detail_enabled = !state.detail_enabled; break;
    case 3: {
      using rengine::lsg::DiagnosticRenderMode;
      const bool enable = state.renderer->diagnostic_mode() != DiagnosticRenderMode::genome_joint_debug;
      state.renderer->set_surface_diagnostic_mode(rengine::lsg::SurfaceDiagnosticMode::none);
      state.renderer->set_diagnostic_mode(enable ? DiagnosticRenderMode::genome_joint_debug
                                                : DiagnosticRenderMode::genome_perspective);
      break;
    }
    case 4: {
      const auto current = state.renderer->camera_state().preset;
      const auto next = current == CameraPreset::full_body ? CameraPreset::portrait
                      : current == CameraPreset::portrait ? CameraPreset::extreme_close_up
                                                         : CameraPreset::full_body;
      state.renderer->set_camera_preset(next);
      break;
    }
    case 5: cycle_mode(state); break;
    case 6: state.renderer->reset_camera_view(); break;
    case 7: {
      using rengine::lsg::PhysiologyPreset;
      const auto current = state.renderer->physiology_preset();
      const auto next = current == PhysiologyPreset::normal ? PhysiologyPreset::exercise
                      : current == PhysiologyPreset::exercise ? PhysiologyPreset::cold
                      : current == PhysiologyPreset::cold ? PhysiologyPreset::hot
                                                          : PhysiologyPreset::normal;
      state.renderer->set_physiology_preset(next);
      break;
    }
    case 8: {
      using rengine::lsg::EyeDiagnosticMode;
      const auto current = state.renderer->eye_diagnostic_mode();
      const auto next = current == EyeDiagnosticMode::normal ? EyeDiagnosticMode::components
                      : current == EyeDiagnosticMode::components ? EyeDiagnosticMode::iris_only
                      : current == EyeDiagnosticMode::iris_only ? EyeDiagnosticMode::cornea_only
                                                                : EyeDiagnosticMode::normal;
      state.renderer->set_eye_diagnostic_mode(next);
      break;
    }
    case 9: {
      using rengine::lsg::LightingPreset;
      const auto current = state.renderer->lighting_preset();
      const auto next = current == LightingPreset::morning ? LightingPreset::noon
                      : current == LightingPreset::noon ? LightingPreset::evening
                      : current == LightingPreset::evening ? LightingPreset::night
                                                           : LightingPreset::morning;
      state.renderer->set_lighting_preset(next);
      print_diagnostics(state, "Time preset changed");
      break;
    }
    case 10: {
      using rengine::lsg::OpticalFilterPreset;
      const auto current = state.renderer->optical_filter_preset();
      const auto next = current == OpticalFilterPreset::clear ? OpticalFilterPreset::tinted
                      : current == OpticalFilterPreset::tinted ? OpticalFilterPreset::polarized_approx
                                                               : OpticalFilterPreset::clear;
      state.renderer->set_optical_filter_preset(next);
      print_diagnostics(state, "Optical filter changed");
      break;
    }
    case 11: state.renderer->toggle_shadow_probe(); break;
    default: break;
  }
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
      if (state != nullptr && wparam == '0') { select_character_ordinal(*state, 0); return 0; }
      if (state != nullptr && wparam == '1') { select_character_ordinal(*state, 1); return 0; }
      if (state != nullptr && wparam == '2') { select_character_ordinal(*state, 2); return 0; }
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
        const int width = static_cast<int>(std::max<LONG>(1, rect.right - rect.left));
        const int height = static_cast<int>(std::max<LONG>(1, rect.bottom - rect.top));
        state->moved = false;
        if (state->character_menu_open) {
          const int menu_row = character_menu_row_from_point(*state, state->down_x, state->down_y, width, height);
          state->pressed_ui_row = menu_row >= 0 ? 100 + menu_row : 99;
          state->hud_candidate = true;
          state->dragging = false;
          SetCapture(window); return 0;
        }
        state->pressed_ui_row = ui_row_from_point(state->down_x, state->down_y, width, height);
        state->hud_candidate = state->pressed_ui_row >= 0;
        state->dragging = !state->hud_candidate;
        state->press_start = std::chrono::steady_clock::now();
        if (state->renderer != nullptr) state->renderer->set_ui_tooltip_row(-1);
        SetCapture(window); return 0;
      }
      break;
    case WM_MOUSEMOVE:
      if (state != nullptr && state->renderer != nullptr) {
        const int x = GET_X_LPARAM(lparam), y = GET_Y_LPARAM(lparam);
        RECT rect{}; GetClientRect(window, &rect);
        const float width = static_cast<float>(std::max<LONG>(1, rect.right - rect.left));
        const float height = static_cast<float>(std::max<LONG>(1, rect.bottom - rect.top));

        if ((wparam & MK_LBUTTON) == 0u) {
          state->renderer->set_ui_tooltip_row(
              ui_row_from_point(x, y, static_cast<int>(width), static_cast<int>(height)));
          return 0;
        }

        state->renderer->set_ui_tooltip_row(-1);
        const float movement = std::hypot(static_cast<float>(x - state->down_x), static_cast<float>(y - state->down_y));
        if (movement > 0.025f * std::min(width, height)) state->moved = true;
        if (state->hud_candidate && movement > 0.025f * std::min(width, height) && !state->character_menu_open) {
          state->hud_candidate = false;
          state->pressed_ui_row = -1;
        }
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
        const int width = static_cast<int>(std::max<LONG>(1, rect.right - rect.left));
        const int height = static_cast<int>(std::max<LONG>(1, rect.bottom - rect.top));
        if (state->character_menu_open && state->pressed_ui_row >= 99) {
          const int menu_row = character_menu_row_from_point(*state, x, y, width, height);
          if (state->pressed_ui_row >= 100 && menu_row >= 0 &&
              state->pressed_ui_row == 100 + menu_row) {
            select_character_ordinal(*state, static_cast<std::uint32_t>(menu_row));
          }
          set_character_menu_open(*state, false);
        } else if (state->hud_candidate && !state->character_menu_open) {
          const int row = ui_row_from_point(x, y, width, height);
          if (row >= 0 && row == state->pressed_ui_row) handle_ui_row(*state, row);
        }
        state->dragging = false;
        state->hud_candidate = false;
        state->moved = false;
        state->pressed_ui_row = -1;
        ReleaseCapture(); return 0;
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
  std::uint32_t requested_profile_ordinal = 0u;
  for (int i = 1; i + 1 < argc; ++i) {
    if (std::string_view{argv[i]} == "--character") {
      const char id = argv[++i][0];
      requested_profile_ordinal = id >= '0' && id <= '7'
          ? static_cast<std::uint32_t>(id - '0') : 0u;
    }
  }
  const HINSTANCE instance = GetModuleHandleW(nullptr);
  constexpr wchar_t kClassName[] = L"RengineLSGPrototypeWindow";
  WNDCLASSW window_class{}; window_class.lpfnWndProc = window_proc; window_class.hInstance = instance;
  window_class.lpszClassName = kClassName; window_class.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
  if (RegisterClassW(&window_class) == 0) { std::cerr << "RegisterClassW failed\n"; return 3; }
  RECT rectangle{0, 0, 1280, 720};
  constexpr DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
  AdjustWindowRect(&rectangle, style, FALSE);
  HWND window = CreateWindowExW(0, kClassName,
      L"Rengine LSG - hold Character button for Base/Female/Ada menu; drag orbit; wheel zoom",
      style, CW_USEDEFAULT, CW_USEDEFAULT, rectangle.right - rectangle.left, rectangle.bottom - rectangle.top,
      nullptr, nullptr, instance, &state);
  if (window == nullptr) { std::cerr << "CreateWindowExW failed\n"; return 4; }
  ShowWindow(window, SW_SHOWDEFAULT);

  if (!renderer.initialize(window)) { std::cerr << "Vulkan 1.2 Win32 initialization failed\n"; DestroyWindow(window); return 5; }
  if (!select_character_ordinal(state, requested_profile_ordinal) &&
      !select_character_ordinal(state, 0u)) {
    std::cerr << "No character profile available in runtime registry\n";
    renderer.shutdown(); DestroyWindow(window); return 6;
  }
  std::cout << "Rengine LSG Windows interactive viewer PASS bootstrap; profiles="
            << renderer.character_profile_count()
            << "; estimated GPU bytes=" << renderer.estimated_gpu_bytes() << "\n";
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
    update_long_press(state);
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
