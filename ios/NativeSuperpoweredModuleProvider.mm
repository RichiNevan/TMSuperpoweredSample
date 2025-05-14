
#import "NativeSuperpoweredModuleProvider.h"
#import <ReactCommon/CallInvoker.h>
#import <ReactCommon/TurboModule.h>
#import "NativeSuperpoweredModule.h"

@implementation NativeSuperpoweredModuleProvider

- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:
    (const facebook::react::ObjCTurboModule::InitParams &)params
{
  return std::make_shared<facebook::react::NativeSuperpoweredModule>(params.jsInvoker);
}

@end
