# Baaaaaaaa: Become the Shepard of your Sound
Baaaaaaaa is my final project for CMPSC 240B, "Digital Audio Programming," a graduate course at UCSB taught by Karl Yerkes. This JUCE audio plugin transforms any sound you give it into shepard tones. This allows you to make shepard scales, shepard-risset glissandi, or shepard melodies with any timbre or character you like.

JUCE allows you to build this plugin in a variety of formats so that it can be inserted into your DAW or audio editor. To build the plugin, first initialize the JUCE and Gamma submodules, which are this project's only external dependencies:

    git submodule update --init --recursive

Then, build with cmake:

    mkdir build
    cmake --build ./build --config Debug --target all --

The exported plugins should appear in baaaJZ/build/plugin_artefacts/Debug.