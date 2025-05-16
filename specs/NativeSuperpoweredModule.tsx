import { TurboModule, TurboModuleRegistry } from "react-native";

export interface Spec extends TurboModule {
  setupAudio(): void; // Takes no args, returns nothing (void)
  cleanupAudio(): void; // Takes no args, returns nothing (void)
  setVolume(volume: number): void; // Takes one number, returns nothing (void)
  startBinaural(
    freqL: number,
    waveformL: number,
    freqR: number,
    waveformR: number,
    initialVolume: number
  ): void; // Takes 5 numbers, returns nothing (void)
  stopBinaural(): void; // Takes no args, returns nothing (void)
  reverseString(input: string): string;
}

export default TurboModuleRegistry.getEnforcing<Spec>(
  "NativeSuperpoweredModule"
);
