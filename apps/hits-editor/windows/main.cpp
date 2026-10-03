#include "dmc_rengine/hits/viewport.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
// clang-format off: Win32 base types must precede dependent SDK headers.
#include <windows.h>
#include <commdlg.h>
#include <windowsx.h>
// clang-format on

using dmc::rengine::hits::viewport::Controller;
using Preset = dmc::rengine::hits::editor::CollisionPreset;
namespace {
Controller editor;
bool scm_mode = false, dragging = false;
POINT previous{};
enum {
    OpenHits = 100,
    OpenScm,
    Save,
    Fit,
    Connected,
    Undo,
    Redo,
    Mode,
    Import,
    Blue,
    Orange,
    Green,
    Red
};
constexpr int toolbar_height = 32;
void error(HWND w, const wchar_t* message) {
    MessageBoxW(w, message, L"HITS Editor", MB_OK | MB_ICONERROR);
}
bool choose(HWND w, bool save, std::filesystem::path& path) {
    wchar_t buffer[32768] = L"";
    OPENFILENAMEW o{};
    o.lStructSize = sizeof(o);
    o.hwndOwner = w;
    o.lpstrFile = buffer;
    o.nMaxFile = 32768;
    o.lpstrFilter = L"HITS / SCM\0*.hits;*.scm\0All files\0*.*\0";
    o.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
    o.lpstrDefExt = save ? L"hits" : nullptr;
    if (!(save ? GetSaveFileNameW(&o) : GetOpenFileNameW(&o)))
        return false;
    path = buffer;
    return true;
}
bool discard(HWND w) {
    return !editor.dirty() || MessageBoxW(w, L"Discard unsaved HITS edits?", L"HITS Editor",
                                          MB_YESNO | MB_ICONQUESTION) == IDYES;
}
void command(HWND w, int id) {
    try {
        if (id == OpenHits || id == OpenScm) {
            if (id == OpenHits && !discard(w))
                return;
            std::filesystem::path p;
            if (!choose(w, false, p))
                return;
            std::ifstream stream(p, std::ios::binary | std::ios::ate);
            const auto size = stream.tellg();
            if (!stream || size <= 0 || size > 256 * 1024 * 1024) {
                error(w, L"Cannot read file (limit 256 MiB).");
                return;
            }
            std::vector<std::byte> bytes(static_cast<std::size_t>(size));
            stream.seekg(0);
            stream.read(reinterpret_cast<char*>(bytes.data()), size);
            if (!stream || !(id == OpenHits ? editor.open_hits(bytes) : editor.open_scm(bytes)))
                error(w, L"Invalid or unsupported resource.");
        } else if (id == Save) {
            auto result = editor.save();
            if (!result.ok()) {
                error(w, L"HITS rebuild failed. No file was written.");
                return;
            }
            std::filesystem::path p;
            if (!choose(w, true, p))
                return;
            // Write a temporary sibling, then replace atomically. Never truncate
            // the selected destination before a successful canonical rebuild.
            wchar_t temporary[MAX_PATH]{};
            if (!GetTempFileNameW(p.parent_path().c_str(), L"HIT", 0, temporary)) {
                error(w, L"Cannot create a temporary file beside the destination.");
                return;
            }
            std::filesystem::path temp = temporary;
            std::ofstream stream(temp, std::ios::binary | std::ios::trunc);
            stream.write(reinterpret_cast<const char*>(result.bytes.data()),
                         static_cast<std::streamsize>(result.bytes.size()));
            stream.close();
            if (!stream || !MoveFileExW(temp.c_str(), p.c_str(),
                                        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
                std::filesystem::remove(temp);
                error(w, L"Unable to save file.");
                return;
            }
            MessageBoxW(w, L"HITS exported successfully.", L"HITS Editor", MB_OK);
        } else if (id == Fit)
            editor.fit();
        else if (id == Connected)
            editor.select_connected();
        else if (id == Undo)
            editor.undo();
        else if (id == Redo)
            editor.redo();
        else if (id == Mode) {
            scm_mode = !scm_mode;
            SetWindowTextW(GetDlgItem(w, Mode), scm_mode ? L"Pick SCM" : L"Pick HITS");
        } else if (id == Import) {
            if (!editor.import_selected_object(Preset::blue_raw_00000001))
                error(w, L"Open HITS and SCM, then pick an SCM object first.");
        } else if (id >= Blue && id <= Red)
            editor.paint(static_cast<Preset>(id - Blue));
        InvalidateRect(w, nullptr, FALSE);
    } catch (const std::exception&) {
        error(w, L"Operation failed. Check file size and available memory.");
    }
}
LRESULT CALLBACK procedure(HWND w, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE: {
        const wchar_t* labels[] = {
            L"Open HITS", L"Open SCM",   L"Export", L"Fit",    L"Connected", L"Undo", L"Redo",
            L"Pick HITS", L"SCM > HITS", L"Blue",   L"Orange", L"Green",     L"Red"};
        for (int i = 0; i < 13; ++i)
            CreateWindowW(L"BUTTON", labels[i], WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, i * 83, 0,
                          82, toolbar_height, w,
                          reinterpret_cast<HMENU>(static_cast<INT_PTR>(OpenHits + i)), nullptr,
                          nullptr);
        return 0;
    }
    case WM_COMMAND:
        command(w, LOWORD(wp));
        return 0;
    case WM_RBUTTONDOWN:
        dragging = true;
        previous = {GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
        SetCapture(w);
        return 0;
    case WM_RBUTTONUP:
        dragging = false;
        ReleaseCapture();
        return 0;
    case WM_CAPTURECHANGED:
        dragging = false;
        return 0;
    case WM_MOUSEMOVE:
        if (dragging) {
            POINT p{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            editor.orbit((p.x - previous.x) * 0.01F, (p.y - previous.y) * 0.01F);
            previous = p;
            InvalidateRect(w, nullptr, FALSE);
        }
        return 0;
    case WM_MOUSEWHEEL:
        editor.zoom(GET_WHEEL_DELTA_WPARAM(wp) > 0 ? 0.85F : 1.18F);
        InvalidateRect(w, nullptr, FALSE);
        return 0;
    case WM_LBUTTONDOWN: {
        RECT r;
        GetClientRect(w, &r);
        editor.pick(static_cast<float>(GET_X_LPARAM(lp)),
                    static_cast<float>(GET_Y_LPARAM(lp) - toolbar_height),
                    static_cast<float>(r.right), static_cast<float>(r.bottom - toolbar_height - 26),
                    scm_mode);
        InvalidateRect(w, nullptr, FALSE);
        return 0;
    }
    case WM_SIZE:
        InvalidateRect(w, nullptr, FALSE);
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(w, &ps);
        RECT r;
        GetClientRect(w, &r);
        HDC mem = CreateCompatibleDC(dc);
        HBITMAP bitmap = CreateCompatibleBitmap(dc, std::max(1L, r.right), std::max(1L, r.bottom));
        auto old = SelectObject(mem, bitmap);
        HBRUSH bg = CreateSolidBrush(RGB(20, 24, 32));
        FillRect(mem, &r, bg);
        DeleteObject(bg);
        for (const auto& t : editor.frame(static_cast<float>(r.right),
                                          static_cast<float>(r.bottom - toolbar_height - 26))) {
            POINT points[3];
            for (int i = 0; i < 3; ++i)
                points[i] = {static_cast<LONG>(std::clamp(t.points[i].x, -1000000.0F, 1000000.0F)),
                             static_cast<LONG>(std::clamp(t.points[i].y, -1000000.0F, 1000000.0F)) +
                                 toolbar_height};
            auto brush =
                CreateSolidBrush(RGB((t.color >> 16) & 255, (t.color >> 8) & 255, t.color & 255));
            auto pen = CreatePen(PS_SOLID, t.selected ? 3 : 1,
                                 t.selected ? RGB(255, 255, 255) : RGB(38, 44, 54));
            auto ob = SelectObject(mem, brush), op = SelectObject(mem, pen);
            Polygon(mem, points, 3);
            SelectObject(mem, ob);
            SelectObject(mem, op);
            DeleteObject(brush);
            DeleteObject(pen);
        }
        SetTextColor(mem, RGB(230, 234, 240));
        SetBkMode(mem, TRANSPARENT);
        std::string text = editor.status() + " | RMB orbit / wheel zoom";
        TextOutA(mem, 8, r.bottom - 22, text.c_str(), static_cast<int>(text.size()));
        BitBlt(dc, 0, toolbar_height, r.right, r.bottom - toolbar_height, mem, 0, toolbar_height,
               SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(bitmap);
        DeleteDC(mem);
        EndPaint(w, &ps);
        return 0;
    }
    case WM_CLOSE:
        if (discard(w))
            DestroyWindow(w);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(w, msg, wp, lp);
}
} // namespace
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int show) {
    WNDCLASSW cls{};
    cls.lpfnWndProc = procedure;
    cls.hInstance = instance;
    cls.lpszClassName = L"RengineHitsEditor";
    cls.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&cls);
    HWND w = CreateWindowW(cls.lpszClassName, L"HITS Editor 0.1 Preview — DMC Rengine",
                           WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
                           1120, 780, nullptr, nullptr, instance, nullptr);
    if (!w)
        return 1;
    ShowWindow(w, show);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
