#include "Renderdoc.h"

#include <Windows.h>
#include <memory>
#include <renderdoc_app.h>
#include <sstream>

class Loader {
public:
  Loader() {
    HMODULE mod = LoadLibraryA("C:\\Program Files\\RenderDoc\\renderdoc.dll");
    if (mod) {
      pfn_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
      if (pfn_GetAPI) {
        pfn_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&api);
      }
    }
  }

  bool IsLoaded() const { return api != nullptr; }
  RENDERDOC_API_1_1_2 *API() { return api; }

private:
  RENDERDOC_API_1_1_2 *api;
  pRENDERDOC_GetAPI pfn_GetAPI;
};

static std::unique_ptr<Loader> loader = nullptr;

// 在创建窗口之前初始化RenderDoc，以确保在窗口创建时能够正确设置捕获窗口
bool Renderdoc::Init() {
  loader = std::make_unique<Loader>();
  return loader && loader->IsLoaded();
}

void Renderdoc::Release() { loader = nullptr; }

void Renderdoc::StartCapture() {
  if (loader && loader->IsLoaded())
    loader->API()->StartFrameCapture(nullptr, nullptr);
}

void Renderdoc::EndCapture() {
  if (loader && loader->IsLoaded())
    loader->API()->EndFrameCapture(nullptr, nullptr);
}

// 在加载GL之后，调用GL命令之前，设置RenderDoc的捕获窗口
bool Renderdoc::SetActiveOpenGLWindow(void *handle) {
  if (handle && loader && loader->IsLoaded()) {
    void *device = wglGetCurrentContext();

    if (device) {
      loader->API()->SetActiveWindow(device, handle);

      return true;
    }
  }

  return false;
}

void Renderdoc::SetCaptureTitle(const std::string &title) {
  if (loader && loader->IsLoaded())
    loader->API()->SetCaptureTitle(title.c_str());
}

void Renderdoc::TriggerCapture() {
  if (loader && loader->IsLoaded())
    loader->API()->TriggerCapture();
}

void Renderdoc::TriggerMultiFrameCapture(uint32_t numFrames) {
  if (loader && loader->IsLoaded())
    loader->API()->TriggerMultiFrameCapture(numFrames);
}

std::optional<std::string> Renderdoc::GetLastCapturePath() {
  if (loader && loader->IsLoaded()) {
    uint32_t numCaptures = loader->API()->GetNumCaptures();
    if (numCaptures == 0)
      return std::nullopt;

    char path[512] = {};
    uint32_t pathLen = sizeof(path);
    uint64_t timestamp = 0;

    if (loader->API()->GetCapture(0, path, &pathLen, &timestamp) == 1) {
      return std::string(path);
    }
  }

  return std::nullopt;
}

bool Renderdoc::LaunchReplayUI(const std::string &capturePath) {
  if (loader && loader->IsLoaded())
    return loader->API()->LaunchReplayUI(
               1, capturePath.empty() ? nullptr : capturePath.c_str()) != 0;

  return false;
}

std::string Renderdoc::GetVersionString() {
  if (loader && loader->IsLoaded()) {
    int major = 0, minor = 0, patch = 0;
    loader->API()->GetAPIVersion(&major, &minor, &patch);
    std::ostringstream oss;
    oss << major << "." << minor << "." << patch;
    return oss.str();
  }

  return {};
}

void Renderdoc::SetOption_APIValidation(bool enable) {
  if (loader && loader->IsLoaded()) {
    loader->API()->SetCaptureOptionU32(eRENDERDOC_Option_APIValidation,
                                       enable ? 1 : 0);
  }
}

void Renderdoc::SetOption_CaptureCallstacks(bool enable) {
  if (loader && loader->IsLoaded()) {
    loader->API()->SetCaptureOptionU32(eRENDERDOC_Option_CaptureCallstacks,
                                       enable ? 1 : 0);
  }
}

void Renderdoc::SetOption_RefAllResources(bool enable) {
  if (loader && loader->IsLoaded()) {
    loader->API()->SetCaptureOptionU32(eRENDERDOC_Option_RefAllResources,
                                       enable ? 1 : 0);
  }
}

void Renderdoc::SetOption_VSync(bool enable) {
  if (loader && loader->IsLoaded()) {
    loader->API()->SetCaptureOptionU32(eRENDERDOC_Option_AllowVSync,
                                       enable ? 1 : 0);
  }
}