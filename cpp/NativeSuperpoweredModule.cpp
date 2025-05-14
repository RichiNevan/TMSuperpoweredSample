#include "NativeSuperpoweredModule.h"

namespace facebook::react {

NativeSuperpoweredModule::NativeSuperpoweredModule(std::shared_ptr<CallInvoker> jsInvoker)
    : NativeSuperpoweredModuleCxxSpec(std::move(jsInvoker)) {}

std::string NativeSuperpoweredModule::reverseString(jsi::Runtime& rt, std::string input) {
  return std::string(input.rbegin(), input.rend());
}

} // namespace facebook::react