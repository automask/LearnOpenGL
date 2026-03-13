#pragma once
#include <optional>
#include <string>

class Renderdoc {
public:
  Renderdoc() = delete;
  Renderdoc(const Renderdoc &) = delete;
  Renderdoc &operator=(const Renderdoc &) = delete;

  static bool Init();
  static void Release();
  static void StartCapture();
  static void EndCapture();
  static bool SetActiveOpenGLWindow(void *handle);
  static void SetCaptureTitle(const std::string &title);
  static void TriggerCapture();
  static void TriggerMultiFrameCapture(uint32_t numFrames);
  static std::optional<std::string> GetLastCapturePath();
  static bool LaunchReplayUI(const std::string &capturePath);
  static std::string GetVersionString();

  static void SetOption_APIValidation(bool enable);
  static void SetOption_CaptureCallstacks(bool enable);
  static void SetOption_RefAllResources(bool enable);
  static void SetOption_VSync(bool enable);
};
