import {TurboModule, TurboModuleRegistry} from 'react-native';

export interface Spec extends TurboModule {
  setupAudio(): void;
  cleanupAudio(): void;
  setVolume(newVolume: number): void;
  startOscillators(freqL: number, waveformL: number, freqR: number, waveformR: number, initialVolume: number): void;
  stopOscillators(): void;
}

export default TurboModuleRegistry.getEnforcing<Spec>(
  'NativeSpecs',
);
