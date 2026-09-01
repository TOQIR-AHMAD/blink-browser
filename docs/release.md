# Releasing

## What a release is

A folder of files and an executable. No service, no scheduled task, no
background updater, nothing added to `Run` keys (PLAN.md §47).

## Steps

1. **Bump the version** in the top-level `CMakeLists.txt` (`project(... VERSION
   x.y.z ...)`). It flows into the executable's version resource, the About
   page and the package names.

2. **Build and run the tests.**

   ```powershell
   cmake --preset windows-release
   cmake --build --preset windows-release
   ctest --preset windows-debug          # the debug preset builds the tests
   ```

3. **Stage the package.**

   ```powershell
   powershell -ExecutionPolicy Bypass -File scripts\package-windows.ps1
   ```

   This produces `dist\PrivacyBrowser-<version>-win64\` and the matching
   portable `.zip`. `windeployqt` collects the Qt runtime, the QML modules and
   the Qt WebEngine process and resources.

4. **Verify the privacy behaviour of the packaged binary**, not of a dev build:

   ```powershell
   powershell -ExecutionPolicy Bypass -File scripts\verify-privacy.ps1 `
       -Executable dist\PrivacyBrowser-<version>-win64\PrivacyBrowser.exe -Seconds 60
   ```

   Browse properly while it runs. The script fails if the session folder
   survives, if the browser reported an incomplete cleanup (exit code 2), or if
   files appear outside the expected set.

   Also run a build configured with `-DPB_NETWORK_AUDIT=ON`, turn the audit on
   in Settings › Advanced, browse, and read the list. Every host should be a
   site you visited, your search provider, your DNS resolver or a filter list
   you added. Anything else is a bug worth stopping the release for.

5. **Licences.** Copy the Qt and Chromium licence texts from your Qt
   installation into `licenses\` in the staged folder, and make sure this
   project's own `LICENSE` exists — see [licensing.md](licensing.md), which is
   still an open decision.

6. **Build the installer** (optional; the portable zip is a complete release on
   its own):

   ```powershell
   iscc /DSourceDir=..\..\dist\PrivacyBrowser-<version>-win64 /DAppVersion=<version> `
        packaging\windows\PrivacyBrowser.iss
   ```

7. **Sign** `PrivacyBrowser.exe` and the installer with an Authenticode
   certificate:

   ```powershell
   signtool sign /fd SHA256 /tr http://timestamp.digicert.com /td SHA256 `
       /f certificate.pfx /p <password> PrivacyBrowser.exe
   ```

   Signing is what lets a user check that a build came from whoever published
   it. It is not implemented in the build system because it needs a private key
   that must not live in a repository.

8. **Publish** the zip, the installer and their SHA-256 checksums, and record
   in the release notes which Qt and Chromium versions the build contains, so a
   reader can tell whether a Chromium security fix is in it.

## Updates

There is no update check, on purpose (PLAN.md §48): a version ping is a beacon
that tells a server when a particular installation is running. Users update by
downloading a new build.

If an update mechanism is ever added, it must be documented before it is
written, and the documentation has to answer: what does the update server learn
about the person checking, how often, and can they turn it off without losing
the ability to install updates by hand?
