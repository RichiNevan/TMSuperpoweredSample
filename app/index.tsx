import { Text, TouchableOpacity, View, StyleSheet } from "react-native";
//import Slider from "@react-native-community/slider";
import { useRef } from "react";
//import NativeSpecs from "@/specs/NativeSpecs";

export default function Index() {
  const volumeRef = useRef(null);
  const freqLRef = useRef(null);
  const freqRRef = useRef(null);

  // const setupAudio = () => {
  //   NativeSpecs.setupAudio();
  // };
  // const cleanupAudio = () => {
  //   NativeSpecs.cleanupAudio();
  // };
  // const setVolume = (newVolume) => {
  //   NativeSpecs.setVolume(newVolume);
  // };
  // const startOscillators = (freqL, waveformL, freqR, waveformR, initialVolume) => {
  //   NativeSpecs.startOscillators(freqL, waveformL, freqR, waveformR, initialVolume);
  // };
  // const stopOscillators = () => {
  //   NativeSpecs.stopOscillators();
  // };

  return (
    <>
      <View style={styles.view}>
        {/* <TouchableOpacity onPress={() => startOscillators(200, 0, 210, 0, 0.5)} style={styles.button}>
          <Text style={styles.buttonText}>Start Oscillators</Text>
        </TouchableOpacity>
        <TouchableOpacity onPress={() => stopOscillators()} style={styles.button}>
          <Text style={styles.buttonText}>Stop Oscillators</Text>
        </TouchableOpacity> */}
      </View>
      <View style={styles.view}>
        <Text style={styles.label}>Volume</Text>
        {/* <Slider
          style={{ width: "70%", height: 40, margin: "auto" }}
          value={volumeRef.current || -30} // Initial value from the voice
          minimumValue={-60}
          maximumValue={-10}
          vertical={true}
          onValueChange={(value) => {
            setVolume(value); // Update the ref value
          }}
        />
          <Text style={styles.label}>Frequency Left</Text>
        <Slider
          style={{ width: "70%", height: 40, margin: "auto" }}
          value={freqLRef.current || -30} // Initial value from the voice
          minimumValue={-60}
          maximumValue={-10}
          vertical={true}
          onValueChange={(value) => {
            freqLRef.current = value; // Update the ref value
          }}
        />
          <Text style={styles.label}>Frequency Right</Text>
        <Slider
          style={{ width: "70%", height: 40, margin: "auto" }}
          value={freqRRef.current || -30} // Initial value from the voice
          minimumValue={-60}
          maximumValue={-10}
          vertical={true}
          onValueChange={(value) => {
            freqRRef.current = value; // Update the ref value
          }} 
        />
           */}
      </View>
    </>
  );
}

const styles = StyleSheet.create({
  button: {
    backgroundColor: "#007AFF",
    padding: 10,
    borderRadius: 5,
    margin: 5,
    alignItems: "center",
    justifyContent: "center",
  },
  buttonText: {
    color: "#FFFFFF",
    fontSize: 16,
  },
  view: {
    flex: 1,
    justifyContent: "center",
    flexDirection: "column",
    alignItems: "center",
  },
});
