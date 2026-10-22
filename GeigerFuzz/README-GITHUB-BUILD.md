# Build the Windows VST3 through GitHub Actions

1. Create a new **public or private** repository on https://github.com/new.
2. Extract this ZIP and upload the **contents** of the `GeigerFuzz` folder to the repository root (including the hidden `.github` folder). Commit to `main` or `master`.
3. Open the repository's **Actions** tab. If prompted, enable Actions.
4. Select **Build Geiger Fuzz VST3 (Windows x64)** and click **Run workflow** (a push to `main`/`master` also starts it automatically).
5. Open the completed workflow run and download the artifact named **Geiger-Fuzz-VST3-Windows-x64** from the **Artifacts** section.
6. Extract the artifact. It should contain `Geiger Fuzz.vst3` (a VST3 bundle folder). Install/copy that folder to `C:\Program Files\Common Files\VST3\` and rescan plugins in your DAW.

The GitHub runner downloads JUCE 8.0.6 during CMake configuration, so the runner needs internet access. If the build fails, open the workflow run and inspect the first red step's log.
