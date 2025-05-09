The app is currently not building. The first goal is to make the android version working (getting the sound to start, and possibly being able to adjust the volume), with ./gradlew installDebug.
In the index.tsx file, everything regarding the native module is commented, as i thought it would help with the building process.
I had to uninstall the react-native-community/slider because was also causing some conflicts.
