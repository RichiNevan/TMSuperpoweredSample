// cpp/MySuperpoweredModule.h

#pragma once

#include <jsi/jsi.h>
#include <NativeMSMSpec.h>  // this path might need adjusting depending on where it's generated

using namespace facebook;

namespace mysuperpoweredmodule {

class MySuperpoweredModule : public NativeMSMSpec<jsi::Value> {
 public:
  MySuperpoweredModule(std::shared_ptr<react::CallInvoker> jsInvoker);

  jsi::Value setupAudio(jsi::Runtime& rt) override;
  jsi::Value cleanupAudio(jsi::Runtime& rt) override;
  jsi::Value setVolume(jsi::Runtime& rt, jsi::Value newVolume) override;
  jsi::Value startBinaural(jsi::Runtime& rt, jsi::Value freqL, jsi::Value waveformL, jsi::Value freqR, jsi::Value waveformR, jsi::Value initialVolume) override;
  jsi::Value stopBinaural(jsi::Runtime& rt) override;
};

} // namespace mysuperpoweredmodule
