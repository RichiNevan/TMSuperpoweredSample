The purpose of this repo changed slightly: 

MAIN BRANCH: 
I would like to get a simple module (the one provided here: https://reactnative.dev/docs/the-new-architecture/pure-cxx-modules) working on both android and iOS.
The repo follows the instruction written in the docs. Only the files' name change, and that is for me to understand the various naming conventions, as well as to give the files the name that they will have in the official project, that will use the Superpowered library.
The android version is working well. The iOS not quite yet.

THE GOAL: making reverseString work fine on iOS as well
THE ISSUE: various errors impeding xcode to properly build

-------------------------------

SPImplementation BRANCH: 
Here is where the superpowered sdk is actually tested. 
As the android turbomodule appears to be set up correctly (the reverseString function works fine in RN), the next step is to implement the library in a way that works.
The NativeSuperpoweredModule.cpp contains some simple functions, that are supposed to be working on RN (as they are implemented in the same way of the reverseString()), but in fact, they are undefined in the RN side. 

THE GOAL: successfully calling setupAudio(), and the other functions from the RN side on android (for iOS, the issue in the main branch must, of course, be solved first)
THE ISSUE: these functions appear undefined in React Native

