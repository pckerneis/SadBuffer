# SadBuffer
Buffer destructive audio plugin

SadBuffer is a free and open-source audio plug-in made with JUCE.

It’s a audio FX designed to mimic digital audio artefacts due to corrupted audio buffers.
It has 3 parameters :
- Freeze mode : when switched on, captures and plays a buffer in loop
- Block size : sets the size (in samples) of the simulated buffer
- CPU overload : adjusts the quantity of randomly skipped (hard cuts) buffers

The Capture button (only accessible via the GUI editor) allow, in Freeze mode, to refresh the frozen buffer.

Enjoy!
