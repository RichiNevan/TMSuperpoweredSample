#include <android/log.h>
#include "SuperpoweredSimple.h"
#include "Superpowered.h"
#include "SuperpoweredAndroidAudioIO.h"
#include "SuperpoweredGenerator.h"
#include <SLES/OpenSLES_Android.h>
#include <memory.h>
#include <jsi/jsi.h> // Include JSI headers
#include "MySuperpoweredModule.h" // Include your generated spec header


// Your core audio processing logic and global variables remain here...
// ... (generatorL, generatorR, audioIO, audioProcessing, rampTo, etc.) ...

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Superpowered", __VA_ARGS__)

// These are global pointers to the Superpowered objects we will use.
Superpowered::Generator *generatorL = nullptr;
Superpowered::Generator *generatorR = nullptr;

SuperpoweredAndroidAudioIO *audioIO = nullptr;

float mainVolume = 0.5f; // Controlled master gain
static bool ramping = true;
static bool shouldStopGenerator = false; // Flag to indicate if the generator should stop after ramping down
static float currentVolume = 0.0f;
const float volumeStep = 0.01f;

// Custom rampTo function
static float rampTo(float current, float target, float step)
{
    if (current < target)
    {
        current += step;
        if (current > target)
            current = target;
    }
    else if (current > target)
    {
        current -= step;
        if (current < target)
            current = target;
    }
    return current;
}

// This is the core audio loop that will be called by the Superpowered SDK many times per second.
// static means, it is only visible in this file
static bool audioProcessing(
    void *clientdata,
    short int *audioIOOutput, // Renamed to audioIOOutput to avoid confusion with global pointer
    int numberOfSamples,
    int samplerate)
{
    if (!generatorL || !generatorR)
    {
        memset(audioIOOutput, 0, numberOfSamples * sizeof(short int) * 2); // Output silence if no generator is present
        return false;                                                      // Indicates that silence is generated
    }

    // --- Generate audio from each oscillator ---
    // Need temporary buffers for the output of each oscillator for this block
    // Using stack allocation for simplicity, assuming numberOfSamples is small (e.g., 512)
    // For very large buffer sizes, consider heap allocation (malloc/new)
    float bufferL[numberOfSamples]; // Buffer for left oscillator output
    float bufferR[numberOfSamples]; // Buffer for right oscillator output

    generatorL->generate(bufferL, numberOfSamples); // Fill bufferL with left generator audio
    generatorR->generate(bufferR, numberOfSamples); // Fill bufferR with right generator audio

    // --- Handle Volume Ramping ---
    // Ramping logic should primarily happen here on the audio thread
    if (ramping) {
        currentVolume = rampTo(currentVolume, mainVolume, volumeStep);
        if (std::abs(currentVolume - mainVolume) < 1e-4) { // Use tolerance for float comparison
             currentVolume = mainVolume; // Snap to target
             ramping = false;
             // LOGI("Ramping finished. Current volume: %.2f", currentVolume); // Optional: log when ramp ends
        }
    }
    // else currentVolume holds the last stable value (mainVolume) --todo: check if this works as expected

    // --- Handle Stop After Ramp Down ---
    if (!ramping && shouldStopGenerator) {
        // This block executes *after* ramping finishes AND if the stop flag was set.
        // Ensure this deletion is safe (e.g., generators aren't being used by the generate calls currently).
        // The Superpowered callback is single-threaded per stream, so this is generally safe here.
        delete generatorL;
        delete generatorR;
        generatorL = nullptr;
        generatorR = nullptr;
        shouldStopGenerator = false; // Reset the stop flag
        LOGI("Generator stopped and deleted after ramp down");
        // Since we just deleted them, return false to indicate silence is generated now.
        memset(audioIOOutput, 0, numberOfSamples * sizeof(short int) * 2); // Ensure silence
        return false;
    }


    // --- Mix the generated audio into the output buffer and apply volume ---
    float stereoBuffer[numberOfSamples * 2]; // Final stereo buffer for mixing

    // Manually interleave the left and right buffers into the stereo buffer
    for (int i = 0; i < numberOfSamples; ++i)
    {
        float leftSample = bufferL[i] * currentVolume;  // Apply volume to left channel
        float rightSample = bufferR[i] * currentVolume; // Apply volume to right channel

        // Clamp to avoid clipping (important after mixing/applying gain)
        if (leftSample > 1.0f)
            leftSample = 1.0f;
        if (leftSample < -1.0f)
            leftSample = -1.0f;
        if (rightSample > 1.0f)
            rightSample = 1.0f;
        if (rightSample < -1.0f)
            rightSample = -1.0f;

        // Copy to stereo channels in the output buffer
        stereoBuffer[i * 2] = leftSample;      // Left channel
        stereoBuffer[i * 2 + 1] = rightSample; // Right channel
    }

    // Convert the final stereo float buffer to short int output
    Superpowered::FloatToShortInt(stereoBuffer, audioIOOutput, numberOfSamples * 2); // Convert float to short int

    // Return true to indicate that the audio processing was successful, and audio is generated
    return true; // Indicates that audio processing was successful
}


namespace facebook::react {

    // Constructor implementation
    MySuperpoweredModule::MySuperpoweredModule(std::shared_ptr<CallInvoker> jsInvoker)
        : NativeMySuperpoweredModuleCxxSpec(jsInvoker) // Call the base constructor
    {
        // Store jsInvoker if needed for async calls back to JS (not needed in this example)
        // this->jsInvoker_ = jsInvoker;
    }

    // Implement JSI methods with correct signatures and argument handling

    jsi::Value MySuperpoweredModule::setupAudio(jsi::Runtime &rt, const jsi::Value* args, size_t count)
    {
        // No arguments expected for setupAudio
        if (count != 0) {
             // Optionally handle unexpected arguments, e.g., log a warning
        }

        // This initializes the Superpowered SDK (Software Development Kit)
        // License key should be managed securely in production.
        Superpowered::Initialize("ExampleLicenseKey-WillExpire-OnNextUpdate");

        // Clean up previous audio engine if exists
        if (audioIO)
        {
            audioIO->stop(); // Stop the audio engine before deleting it
            delete audioIO;
            audioIO = nullptr; // Set to nullptr to avoid dangling pointer
            LOGI("Previous Audio engine stopped and cleaned up");
        }

        // Start the audio engine
        // Consider making sample rate and buffer size configurable if needed
        int sampleRate = 44100;
        int bufferSize = 512; // 0 = default, but 512 is common for low latency
        audioIO = new SuperpoweredAndroidAudioIO(
            sampleRate,
            bufferSize,
            false,                  // Enable input?
            true,                   // Enable output?
            audioProcessing,        // Audio callback function pointer
            nullptr,                // Custom data (we don't need this)
            -1,                     // Input stream type (not used here)
            SL_ANDROID_STREAM_MEDIA // Output stream type
        );

        if (audioIO)
        {
            audioIO->start();
            LOGI("Audio engine started successfully at %dHz with buffer %d", sampleRate, bufferSize);
        }
        else
        {
            LOGI("Failed to start audio engine");
            // You might want to throw a JSI exception here or return a specific value
            // throw jsi::JSError(rt, "Failed to start audio engine"); // Example of throwing
        }

        // Reset audio state
        mainVolume = 0.0f; // Start muted
        currentVolume = 0.0f; // Ensure audio thread starts from 0
        ramping = false; // Initially no ramping needed unless volume is changed
        shouldStopGenerator = false; // Reset stop flag

        // Need to return a jsi::Value
        return jsi::Value::undefined(); // Return undefined as there's no specific value to return
    }

    jsi::Value MySuperpoweredModule::cleanupAudio(jsi::Runtime &rt, const jsi::Value* args, size_t count)
    {
         if (count != 0) {
             // Log warning for unexpected args
         }

        // This function is called when the app is closed or audio is no longer needed.
        // It cleans up the audio engine and any other resources.

        // It's safer to signal the audio thread to stop generators first,
        // especially if cleanup might be called while audio is running.
        // However, stopping audioIO first is the standard Superpowered way.
        // Stopping audioIO will stop the audio processing thread.
        // Then it's safe to delete generators.

        if (audioIO)
        {
            audioIO->stop(); // Stop the audio engine (stops the audio thread)
            delete audioIO;  // Clean up the audio engine
            audioIO = nullptr;
            LOGI("Audio engine stopped and cleaned up");
        }

        // Now it's safe to delete generators as the audio thread is stopped
        if (generatorL)
        {
            delete generatorL; // Clean up the left generator
            generatorL = nullptr;
        }
        if (generatorR)
        {
            delete generatorR; // Clean up the right generator
            generatorR = nullptr;
        }
        shouldStopGenerator = false; // Reset the stop flag

        return jsi::Value::undefined(); // Return undefined
    }

    jsi::Value MySuperpoweredModule::setVolume(jsi::Runtime &rt, const jsi::Value* args, size_t count)
    {
        if (count != 1 || !args[0].isNumber()) {
            // Handle incorrect arguments
             throw jsi::JSError(rt, "setVolume requires one number argument (0.0 to 1.0)");
        }

        // Get the volume from the JSI argument
        float newVolume = args[0].getNumber();

        // Clamp volume to 0.0 - 1.0 range
        if (newVolume < 0.0f) newVolume = 0.0f;
        if (newVolume > 1.0f) newVolume = 1.0f;

        // Set the target volume and start ramping
        mainVolume = newVolume;
        ramping = true;

        // Don't log frequently in the JS thread method, prefer audio thread or less frequent logs.
        // LOGI("Set target volume: %.2f", mainVolume); // Log target volume

        return jsi::Value::undefined(); // Return undefined
    }

    jsi::Value MySuperpoweredModule::startOscillators(
        jsi::Runtime &rt,
        const jsi::Value* args,
        size_t count)
    {
        // Expecting 5 arguments: freqL, waveformL, freqR, waveformR, initialVolume
        if (count != 5 || !args[0].isNumber() || !args[1].isNumber() || !args[2].isNumber() || !args[3].isNumber() || !args[4].isNumber())
        {
             throw jsi::JSError(rt, "startOscillators requires 5 number arguments (freqL, waveformL, freqR, waveformR, initialVolume)");
        }

        // Get arguments and convert from jsi::Value to C++ types
        float freqL = args[0].getNumber();
        int waveformL = args[1].getNumber(); // Expecting an integer representing the enum value
        float freqR = args[2].getNumber();
        int waveformR = args[3].getNumber(); // Expecting an integer
        float initialVolume = args[4].getNumber(); // Expecting 0.0 to 1.0

        // Clean up previous generators if they exist
        if (generatorL)
        {
            delete generatorL;
            generatorL = nullptr; // Ensure pointer is null after deletion
        }
        if (generatorR)
        {
            delete generatorR;
            generatorR = nullptr; // Ensure pointer is null after deletion
        }

        shouldStopGenerator = false; // Reset the stop flag

        // Create new generators
        int samplerate = 44100; // Default, or get from audioIO if available and started
        if (audioIO) {
            // samplerate = audioIO->getSampleRate(); // Use actual sample rate if audioIO is ready
        }

        // Basic validation for waveform types if needed
        if (waveformL < 0 || waveformL >= Superpowered::Generator::GeneratorShape::NumGeneratorShapes ||
            waveformR < 0 || waveformR >= Superpowered::Generator::GeneratorShape::NumGeneratorShapes) {
             throw jsi::JSError(rt, "Invalid waveform shape value");
        }


        generatorL = new Superpowered::Generator(samplerate, static_cast<Superpowered::Generator::GeneratorShape>(waveformL));
        generatorR = new Superpowered::Generator(samplerate, static_cast<Superpowered::Generator::GeneratorShape>(waveformR));

        // Set frequencies
        generatorL->frequency = freqL;
        generatorR->frequency = freqR;

        // reset phase (optional, depending on desired behavior)
        generatorL->reset();
        generatorR->reset();

        // Set initial volume and start ramping
        // Clamp initial volume
        if (initialVolume < 0.0f) initialVolume = 0.0f;
        if (initialVolume > 1.0f) initialVolume = 1.0f;

        mainVolume = initialVolume; // Target volume
        currentVolume = 0.0f;       // Start ramp from mute for fade-in
        ramping = true;             // Start ramping

        LOGI("Started oscillators with frequencies: L=%.2f, R=%.2f, target volume=%.2f", freqL, freqR, initialVolume);

        return jsi::Value::undefined(); // Return undefined
    }

    jsi::Value MySuperpoweredModule::stopOscillators(jsi::Runtime &rt, const jsi::Value* args, size_t count)
    {
        if (count != 0) {
             // Log warning for unexpected args
        }

        if (generatorL && generatorR)
        {
            // Ramp down to silence
            mainVolume = 0.0f;          // Target volume is silence
            ramping = true;             // Start ramping down
            shouldStopGenerator = true; // Set the flag to stop/delete generators AFTER ramping finishes

            LOGI("Stopping oscillator, initiating ramp down to silence");
        }
        else
        {
            LOGI("No generators to stop");
            // Clean up immediately if they somehow weren't properly deleted before
            if (generatorL) { delete generatorL; generatorL = nullptr; }
            if (generatorR) { delete generatorR; generatorR = nullptr; }
            shouldStopGenerator = false; // Ensure flag is false
        }

        return jsi::Value::undefined(); // Return undefined
    }

    // --- Add the Factory Function ---
    // This function will be called by React Native to create an instance of your module.
    std::shared_ptr<TurboModule> MySuperpoweredModule::getModule(
        const std::string& moduleName,
        const ObjCTurboManager& objcManager) { // Use appropriate manager type based on platform/setup
        // In C++, you typically define this factory function outside the class
        // It's usually in a separate .cpp file or at the end of this one.
        // The signature might vary slightly based on how you configure Turbo Modules (using CodeGen or not, specific RN version).
        // A common C++ factory signature looks like this:
        /*
        std::shared_ptr<TurboModule> getModule(
           const std::string& name,
           std::shared_ptr<CallInvoker> jsInvoker);
        */
        // Let's assume the simpler CallInvoker signature for the example:
        // You need to define this function *outside* the class, but within the namespace.
        // Example:

        /*
        // Needs to be outside the class declaration
        std::shared_ptr<TurboModule> getModule(
            const std::string& name,
            std::shared_ptr<CallInvoker> jsInvoker) {
            if (name == "MySuperpoweredModule") { // Or whatever your module name is
                return std::make_shared<MySuperpoweredModule>(jsInvoker);
            }
            return nullptr;
        }
        */
       // NOTE: The actual factory function signature and where it lives depends on your specific Turbo Module setup (manual or codegen).
       // The above is a common pattern, but check your React Native version's documentation or generated code examples.

       // For this example, we'll just return nullptr here as the factory function isn't part of the class definition.
       // You will need to add the actual factory function definition separately.
        return nullptr; // Placeholder - actual factory is defined elsewhere
    }


} // namespace facebook::react

// Add the actual factory function implementation outside the class body, but often within the namespace
/*
namespace facebook::react {

std::shared_ptr<TurboModule> MySuperpoweredModule_Module_getModule(
    const std::string& moduleName,
    const ObjCTurboManager& objCTurboManager, // Or other manager types
    const std::shared_ptr<CallInvoker>& jsInvoker) {

    if (moduleName == "MySuperpoweredModule") { // Match the name used in JS/Spec
        return std::make_shared<MySuperpoweredModule>(jsInvoker);
    }
    return nullptr;
}

} // namespace facebook::react
*/
// The exact name (MySuperpoweredModule_Module_getModule) and signature can depend on codegen/RN version.
// You'll need to integrate this factory function into your native module registration (e.g., in a ReactPackage in Java/Kotlin).