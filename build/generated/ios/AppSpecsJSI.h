/**
 * This code was generated by [react-native-codegen](https://www.npmjs.com/package/react-native-codegen).
 *
 * Do not edit this file as changes may cause incorrect behavior and will be lost
 * once the code is regenerated.
 *
 * @generated by codegen project: GenerateModuleH.js
 */

#pragma once

#include <ReactCommon/TurboModule.h>
#include <react/bridging/Bridging.h>

namespace facebook::react {


  class JSI_EXPORT NativeSpecsCxxSpecJSI : public TurboModule {
protected:
  NativeSpecsCxxSpecJSI(std::shared_ptr<CallInvoker> jsInvoker);

public:
  virtual void setupAudio(jsi::Runtime &rt) = 0;
  virtual void cleanupAudio(jsi::Runtime &rt) = 0;
  virtual void setVolume(jsi::Runtime &rt, double newVolume) = 0;
  virtual void startOscillators(jsi::Runtime &rt, double freqL, double waveformL, double freqR, double waveformR, double initialVolume) = 0;
  virtual void stopOscillators(jsi::Runtime &rt) = 0;

};

template <typename T>
class JSI_EXPORT NativeSpecsCxxSpec : public TurboModule {
public:
  jsi::Value create(jsi::Runtime &rt, const jsi::PropNameID &propName) override {
    return delegate_.create(rt, propName);
  }

  std::vector<jsi::PropNameID> getPropertyNames(jsi::Runtime& runtime) override {
    return delegate_.getPropertyNames(runtime);
  }

  static constexpr std::string_view kModuleName = "NativeSpecs";

protected:
  NativeSpecsCxxSpec(std::shared_ptr<CallInvoker> jsInvoker)
    : TurboModule(std::string{NativeSpecsCxxSpec::kModuleName}, jsInvoker),
      delegate_(reinterpret_cast<T*>(this), jsInvoker) {}


private:
  class Delegate : public NativeSpecsCxxSpecJSI {
  public:
    Delegate(T *instance, std::shared_ptr<CallInvoker> jsInvoker) :
      NativeSpecsCxxSpecJSI(std::move(jsInvoker)), instance_(instance) {

    }

    void setupAudio(jsi::Runtime &rt) override {
      static_assert(
          bridging::getParameterCount(&T::setupAudio) == 1,
          "Expected setupAudio(...) to have 1 parameters");

      return bridging::callFromJs<void>(
          rt, &T::setupAudio, jsInvoker_, instance_);
    }
    void cleanupAudio(jsi::Runtime &rt) override {
      static_assert(
          bridging::getParameterCount(&T::cleanupAudio) == 1,
          "Expected cleanupAudio(...) to have 1 parameters");

      return bridging::callFromJs<void>(
          rt, &T::cleanupAudio, jsInvoker_, instance_);
    }
    void setVolume(jsi::Runtime &rt, double newVolume) override {
      static_assert(
          bridging::getParameterCount(&T::setVolume) == 2,
          "Expected setVolume(...) to have 2 parameters");

      return bridging::callFromJs<void>(
          rt, &T::setVolume, jsInvoker_, instance_, std::move(newVolume));
    }
    void startOscillators(jsi::Runtime &rt, double freqL, double waveformL, double freqR, double waveformR, double initialVolume) override {
      static_assert(
          bridging::getParameterCount(&T::startOscillators) == 6,
          "Expected startOscillators(...) to have 6 parameters");

      return bridging::callFromJs<void>(
          rt, &T::startOscillators, jsInvoker_, instance_, std::move(freqL), std::move(waveformL), std::move(freqR), std::move(waveformR), std::move(initialVolume));
    }
    void stopOscillators(jsi::Runtime &rt) override {
      static_assert(
          bridging::getParameterCount(&T::stopOscillators) == 1,
          "Expected stopOscillators(...) to have 1 parameters");

      return bridging::callFromJs<void>(
          rt, &T::stopOscillators, jsInvoker_, instance_);
    }

  private:
    friend class NativeSpecsCxxSpec;
    T *instance_;
  };

  Delegate delegate_;
};

} // namespace facebook::react
