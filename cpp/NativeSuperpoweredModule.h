#pragma once

#include <AppSpecsJSI.h>

#include <memory>
#include <string>

namespace facebook::react {

class NativeSuperpoweredModule : public NativeSuperpoweredModuleCxxSpec<NativeSuperpoweredModule> {
public:
  NativeSuperpoweredModule(std::shared_ptr<CallInvoker> jsInvoker);

  void setupAudio(jsi::Runtime& rt);
  
  void cleanupAudio(jsi::Runtime& rt);
  
  void setVolume(jsi::Runtime& rt, float volume);
  
  void startBinaural(jsi::Runtime& rt, float freqL, int waveformL, float freqR, int waveformR, float initialVolume);
  
  void stopBinaural(jsi::Runtime& rt);

  std::string reverseString(jsi::Runtime& rt, std::string input);
};

} // namespace facebook::react