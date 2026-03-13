#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

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
  static void DiscardFrameCapture();
  static bool IsFrameCapturing();

  static void TriggerCapture();
  static void TriggerMultiFrameCapture(uint32_t numFrames);
  static bool ShowReplayUI();
  static bool LaunchReplayUI(const std::string &capturePath);

  static std::optional<std::string> GetLastCapturePath();
  static std::string GetVersionString();

  static uint32_t GetOverlayMask();
  static void EnableOverlay(bool enable);
  static void SetOption_APIValidation(bool enable);
  static void SetOption_CaptureCallstacks(bool enable);
  static void SetOption_RefAllResources(bool enable);
  static void SetOption_VSync(bool enable);
  static void SetCaptureKeys(const std::vector<uint32_t> &keys);
};
