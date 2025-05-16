#include <android/log.h>
#include "NativeSuperpoweredModule.h"
#include "SuperpoweredSimple.h"
#include "Superpowered.h"
#include "SuperpoweredAndroidAudioIO.h"
#include "SuperpoweredGenerator.h"
#include <SLES/OpenSLES_Android.h>
#include <memory.h>
#include <jsi/jsi.h> 

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
          //  LOGI("Ramping finished. Current volume: %.2f", currentVolume); // Optional: log when ramp ends
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
      // LOGI("Generator stopped and deleted after ramp down");
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

NativeSuperpoweredModule::NativeSuperpoweredModule(std::shared_ptr<CallInvoker> jsInvoker)
    : NativeSuperpoweredModuleCxxSpec(std::move(jsInvoker)) {}


void NativeSuperpoweredModule::setupAudio(jsi::Runtime &rt)
{
    // This initializes the Superpowered SDK (Software Development Kit)
    // License key should be managed securely in production.
    // TODO: Consider making the license key configurable or managed better
    Superpowered::Initialize("ExampleLicenseKey-WillExpire-OnNextUpdate");
    // LOGI("Superpowered SDK Initialized"); // Added log for clarity

    // Clean up previous audio engine if exists
    if (audioIO)
    {
        audioIO->stop(); // Stop the audio engine before deleting it
        delete audioIO;
        audioIO = nullptr; // Set to nullptr to avoid dangling pointer
        // LOGI("Previous Audio engine stopped and cleaned up");
    }

    // Start the audio engine
    // Consider making sample rate and buffer size configurable if needed (add parameters to the function signature)
    int sampleRate = 44100; // TODO: Make configurable?
    int bufferSize = 512;   // TODO: Make configurable? // 0 = default, but 512 is common for low latency

    // Assuming audioProcessing is your static/global callback function,
    // or bound correctly if it's a member function.
    // SuperpoweredAndroidAudioIO expects a specific function pointer signature.
    // If audioProcessing is a class member, you might need a static helper
    // or store a 'this' pointer (user data) to call the member.
    // The original code implies audioProcessing might be a static method or global.
    audioIO = new SuperpoweredAndroidAudioIO(
        sampleRate,
        bufferSize,
        false,                  // Enable input? (Hardcoded false) TODO: Make configurable?
        true,                   // Enable output? (Hardcoded true) TODO: Make configurable?
        audioProcessing,        // Audio callback function pointer
        nullptr,                // Custom data (we don't need this, matches original)
        -1,                     // Input stream type (not used here, matches original)
        SL_ANDROID_STREAM_MEDIA // Output stream type (Hardcoded) TODO: Make configurable?
    );

    if (audioIO)
    {
        audioIO->start();
        // LOGI("Audio engine started successfully at %dHz with buffer %d", sampleRate, bufferSize);
    }
    else
    {
        // LOGI("Failed to start audio engine");
        // In a Turbo Module context, throwing a JSI exception is the standard way
        // to signal an error back to JavaScript.
        throw jsi::JSError(rt, "Failed to create or start Superpowered audio engine");
    }

    // Reset audio state - assuming these are members of NativeSuperpoweredModule
    mainVolume = 0.0f; // Start muted
    currentVolume = 0.0f; // Ensure audio thread starts from 0
    ramping = false; // Initially no ramping needed unless volume is changed
    shouldStopGenerator = false; // Reset stop flag

    // Need to return a jsi::Value
    // Returning undefined() or null() is common for functions without a meaningful return value.
    // return jsi::Value::undefined(); Remove this line if the function is running
}
void NativeSuperpoweredModule::cleanupAudio(jsi::Runtime &rt)
{
    // It's safer to signal the audio thread to stop generators first,
    // especially if cleanup might be called while audio is running.
    // However, stopping audioIO first is the standard Superpowered way
    // and ensures the audio thread is stopped before deleting objects it might use.

    // Assuming shouldStopGenerator is a flag used by your audioProcessing callback
    // to gracefully stop generator output. Setting it *before* stopping audioIO
    // allows the audio thread one more loop to potentially see it, but stopping
    // audioIO is the guaranteed way to stop the audio thread. The order here
    // matches your original logic, stopping audioIO first.

    if (audioIO)
    {
        audioIO->stop(); // Stop the audio engine (stops the audio thread)
        delete audioIO;  // Clean up the audio engine
        audioIO = nullptr;
        // LOGI("Audio engine stopped and cleaned up");
    }

    // Now it should be safe to delete generators as the audio thread is stopped
    if (generatorL)
    {
        delete generatorL; // Clean up the left generator
        generatorL = nullptr;
        // LOGI("generatorL cleaned up"); // Added log
    }
    if (generatorR)
    {
        delete generatorR; // Clean up the right generator
        generatorR = nullptr;
        // LOGI("generatorR cleaned up"); // Added log
    }
    // Reset the stop flag, although it might not be strictly necessary
    // after deleting the generators and stopping the audio thread.
    shouldStopGenerator = false;


    // Need to return a jsi::Value
    // Returning undefined() or null() is common for functions without a meaningful return value.
    // return jsi::Value::undefined(); Remove this line if the function is running
}

void NativeSuperpoweredModule::setVolume(jsi::Runtime &rt, float volume)
{
    // --- Arguments handling is now implicitly handled by the Turbo Modules binding layer ---
    // The binding layer for this specific function will:
    // 1. Check that EXACTLY ONE argument is passed from JavaScript.
    // 2. Check that the argument is a number.
    // 3. Convert the JavaScript number to a C++ float and pass it as the 'volume' parameter.
    // If any of these checks fail, the binding layer will likely throw a JSI error
    // BEFORE calling this function.
    // check or the manual throwing of JSError for bad arguments.

    // The volume value is now available directly in the 'volume' parameter.

    // Clamp volume to 0.0 - 1.0 range
    // This clamping logic should remain, as it's specific application logic,
    // not just basic type conversion or argument validation.
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;

    // Set the target volume and start ramping
    mainVolume = volume; // Use the parameter directly
    ramping = true;

    // Don't log frequently in the JS thread method, prefer audio thread or less frequent logs.
    // LOGI("Set target volume: %.2f", mainVolume); // Log target volume

    // Need to return a jsi::Value
    // Returning undefined() or null() is common for functions without a meaningful return value.
    // return jsi::Value::undefined(); Remove this line if the function is running
}

void NativeSuperpoweredModule::startBinaural(
  jsi::Runtime &rt,
  float freqL,
  int waveformL, // Turbo Modules can typically convert JS Number to int
  float freqR,
  int waveformR, // Turbo Modules can typically convert JS Number to int
  float initialVolume)
{
  // --- Arguments handling is now implicitly handled by the Turbo Modules binding layer ---
  // The binding layer for this specific function will:
  // 1. Check that EXACTLY FIVE arguments are passed from JavaScript.
  // 2. Check that ALL FIVE arguments are numbers.
  // 3. Convert the JavaScript numbers to C++ float or int and pass them as parameters.
  // If any of these checks fail, the binding layer will likely throw a JSI error
  // BEFORE calling this function. You no longer need the manual argument count,
  // type checking, and initial JSError throw based on these.

  // Get arguments and convert from jsi::Value to C++ types - NOT NEEDED ANYMORE!
  // The values are already in the parameters: freqL, waveformL, freqR, waveformR, initialVolume

  // Clean up previous generators if they exist
  if (generatorL)
  {
      delete generatorL;
      generatorL = nullptr; // Ensure pointer is null after deletion
      // LOGI("Previous generatorL cleaned up"); // Added log
  }
  if (generatorR)
  {
      delete generatorR;
      generatorR = nullptr; // Ensure pointer is null after deletion
      // LOGI("Previous generatorR cleaned up"); // Added log
  }

  shouldStopGenerator = false; // Reset the stop flag

  // Create new generators
  int samplerate = 44100; // Default
  // Consider getting actual samplerate from audioIO if reliable and available
  if (audioIO) {
      // samplerate = audioIO->getSampleRate();
      // Be cautious: if audioIO is not started yet, getSampleRate() might not be valid or crash.
      // Using a fixed default like 44100Hz is often simpler unless you need dynamic sample rate.
  }
  // LOGI("Using samplerate: %d", samplerate); // Log samplerate being used

  // Basic validation for waveform types - KEEP this, as it's logic based on the *value's meaning*
  // within Superpowered, not just a basic type check.
  // If you expect specific enum values, checking their valid range is important.

  // Create generators using the parameters
  generatorL = new Superpowered::Generator(samplerate, static_cast<Superpowered::Generator::GeneratorShape>(waveformL));
  generatorR = new Superpowered::Generator(samplerate, static_cast<Superpowered::Generator::GeneratorShape>(waveformR));

  // Set frequencies using the parameters
  generatorL->frequency = freqL;
  generatorR->frequency = freqR;

  // reset phase (optional, depending on desired behavior)
  generatorL->reset();
  generatorR->reset();

  // Set initial volume and start ramping
  // Clamp initial volume - KEEP this, it's application logic
  if (initialVolume < 0.0f) initialVolume = 0.0f;
  if (initialVolume > 1.0f) initialVolume = 1.0f;

  mainVolume = initialVolume; // Target volume
  currentVolume = 0.0f;       // Start ramp from mute for fade-in
  ramping = true;             // Start ramping

  // LOGI("Started binaural with frequencies: L=%.2f, R=%.2f, target volume=%.2f, waveforms: L=%d, R=%d",
      //  freqL, freqR, initialVolume, waveformL, waveformR);

  // Need to return a jsi::Value
  // Returning undefined() or null() is common for functions without a meaningful return value.
  // return jsi::Value::undefined(); Remove this line if the function is running
}

void NativeSuperpoweredModule::stopBinaural(jsi::Runtime &rt)
{
    // --- Arguments handling is now implicitly handled by the Turbo Modules binding layer ---
    // The binding layer for this specific function will check that NO arguments are passed
    // from JavaScript. If any ARE passed, the binding layer will likely throw an error
    // BEFORE calling this function. You no longer need the 'if (count != 0)' check here.

    if (generatorL && generatorR)
    {
        // Ramp down to silence
        mainVolume = 0.0f;          // Target volume is silence
        ramping = true;             // Start ramping down
        // Set the flag to indicate that generators should be stopped/deleted
        // *after* the volume ramp finishes (presumably handled in audioProcessing)
        shouldStopGenerator = true;

        // LOGI("Stopping binaural, initiating ramp down to silence");
    }
    else
    {
        // LOGI("No active generators to stop");
        // Clean up immediately if they somehow weren't properly deleted before
        // This case might happen if startBinaural failed partially, or cleanupAudio was called already.
        if (generatorL) {
            delete generatorL;
            generatorL = nullptr;
            // LOGI("Found and deleted generatorL");
        }
        if (generatorR) {
            delete generatorR;
            generatorR = nullptr;
            // LOGI("Found and deleted generatorR");
        }
        // Ensure flag is false if no generators were active or they were just deleted
        shouldStopGenerator = false;
    }

    // Need to return a jsi::Value
    // Returning undefined() or null() is common for functions without a meaningful return value.
    // return jsi::Value::undefined(); Remove this line if the function is running
}

std::string NativeSuperpoweredModule::reverseString(jsi::Runtime& rt, std::string input) {
  return std::string(input.rbegin(), input.rend());
}

} // namespace facebook::react