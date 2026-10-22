# Geiger Fuzz — VST3 source project

An original digital fuzz effect inspired by the broad concept of aggressive wavetable/bit-reduction distortion. It is **not** an exact recreation of the WMD Geiger Counter circuit, firmware, or tables.

## Features
- Drive / nonlinear gain
- Four original digital transfer modes
- Bit-depth reduction (1–16 bit)
- Sample-and-hold rate reduction
- Tone low-pass filter
- Dry/wet mix and output gain
- Stereo and mono processing
- Parameter state save/restore
- VST3 and Standalone formats

## Build on Windows 10/11 x64

Requirements:
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.22 or newer
- Git
- Internet access for the first configure (CMake fetches JUCE 8.0.6)

From an x64 Native Tools Command Prompt or PowerShell:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target GeigerFuzz_VST3
```

The VST3 bundle should be created under a path similar to:
`build/GeigerFuzz_artefacts/Release/VST3/Geiger Fuzz.vst3`

The Standalone app is also available as a separate build target:
```powershell
cmake --build build --config Release --target GeigerFuzz_Standalone
```

Install by copying the entire `.vst3` bundle folder to:
`C:\\Program Files\\Common Files\\VST3\\`

Then rescan plugins in your DAW. Some hosts require a restart.

## Notes
- This source archive does not include a precompiled Windows binary. A genuine Windows VST3 binary must be compiled with a Windows toolchain.
- If the compiler reports missing Windows SDK or CMake components, modify the Visual Studio installation to include them.
- The effect uses no external audio files.
