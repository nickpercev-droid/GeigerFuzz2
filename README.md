# HELLFURNACE FUZZ VST3

Extreme guitar fuzz VST3 for Windows x64, built with JUCE.

## Controls
- FUZZ: gain and saturation intensity
- CRUSH: bit-depth reduction and sample-rate destruction
- GATE: aggressive noise gate
- BIAS: asymmetric clipping character
- TONE: high-frequency bite
- BODY: low-frequency weight
- MIX: dry/wet
- OUTPUT: output level
- MODE: Classic / Searing / Chaos Max voicing
- ON: effect bypass switch

## Build on GitHub
1. Create a new GitHub repository.
2. Upload the contents of this folder to the repository root. Make sure `.github/workflows/build.yml` is included.
3. Open the **Actions** tab.
4. Select **Build HELLFURNACE FUZZ VST3 (Windows x64)** and press **Run workflow** (or push a commit).
5. Open the completed run and download the artifact **HELLFURNACE-FUZZ-Windows-x64**.
6. Extract the ZIP. Copy `HELLFURNACE FUZZ.vst3` into:
   `C:\Program Files\Common Files\VST3\`
7. Rescan plugins in your DAW.

The first build downloads JUCE on the GitHub Windows runner. This repository does not contain JUCE itself.
