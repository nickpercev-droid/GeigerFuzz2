# Geiger Counter Pro (Behavioral) — VST3 for Windows x64

This is an independent behavioral recreation of the published Geiger Counter Pro signal-flow concept. It is **not** an official WMD product and does not contain WMD firmware or proprietary factory wavetable data. The 512 tables are procedurally generated original tables, arranged as 32 banks x 16 tables.

## Implemented controls

Gain, Tone + Tone On, Mix + Dry, Samples + Fine, Bits (1–12), Mask, Pre WT, Bank (0–31), Table (0–15), Morph, Filter, Level, Master, Activate, and a random patch button. The processing path models an analog-style preamp, optional tone stage, sample-rate hold, 12-bit quantization, pre/post wavetable bit reduction, wavetable shaping, low-pass filter, mix, and output level.

## Build on GitHub (Windows x64)

1. Extract the ZIP.
2. Upload the **contents** of this folder to the root of your `GeigerFuzz2` repository. Include the hidden `.github` directory. Replace the old Geiger Fuzz project files.
3. Commit to `main` or `master`.
4. Open **Actions** and run **Build Geiger Counter Pro VST3 (Windows x64)**.
5. When it succeeds, download the artifact `Geiger-Counter-Pro-VST3-Windows-x64` from the workflow run.
6. Extract the artifact and copy the `.vst3` bundle folder to `C:\Program Files\Common Files\VST3\`, then rescan plugins in your DAW.

The GitHub runner fetches JUCE 8.0.6 during configuration, so GitHub-hosted build runners need internet access.

## Local build

Requires Visual Studio 2022 with Desktop development with C++, CMake 3.22+, and internet access:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target GeigerCounterPro_VST3 --parallel
```

## Accuracy note

WMD's manual describes 32 banks of 16 tables, 12-bit processing, sample-rate reduction, optional pre-table bit reduction, morphing, filter, dry/wet, and level/master stages. This project follows that documented control architecture, but the exact proprietary table contents, firmware behavior, and analog circuit response are not available here. Treat this as a functional behavioral interpretation, not a bit-perfect clone.
