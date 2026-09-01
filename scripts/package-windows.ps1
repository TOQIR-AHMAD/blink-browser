# Builds a release and lays out a distributable Windows package (PLAN.md 47).
#
#   powershell -ExecutionPolicy Bypass -File scripts\package-windows.ps1
#
# Produces:
#   dist\PrivacyBrowser-<version>-win64\      the portable layout
#   dist\PrivacyBrowser-<version>-win64.zip   the portable download
#
# The installer is built separately from packaging\windows\PrivacyBrowser.iss;
# see docs/release.md.
#
# Nothing here installs a service, a scheduled task or an updater: the package
# is a folder of files and an executable (PLAN.md 47, 48).

param(
    [string]$Preset = 'windows-release',
    [switch]$SkipBuild
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

if (-not $env:QT_ROOT_DIR) {
    throw "QT_ROOT_DIR is not set. See docs/build-windows.md."
}

$windeployqt = Join-Path $env:QT_ROOT_DIR 'bin\windeployqt.exe'
if (-not (Test-Path $windeployqt)) {
    throw "windeployqt.exe not found at $windeployqt"
}

if (-not $SkipBuild) {
    Push-Location $root
    try {
        cmake --preset $Preset
        if ($LASTEXITCODE -ne 0) { throw "cmake configure failed" }
        cmake --build --preset $Preset
        if ($LASTEXITCODE -ne 0) { throw "build failed" }
    } finally {
        Pop-Location
    }
}

$binary = Join-Path $root "build\$Preset\bin\PrivacyBrowser.exe"
if (-not (Test-Path $binary)) {
    throw "Built executable not found at $binary"
}

$version = (Get-Content (Join-Path $root 'CMakeLists.txt') |
    Select-String -Pattern 'VERSION\s+(\d+\.\d+\.\d+)' |
    Select-Object -First 1).Matches[0].Groups[1].Value

$stage = Join-Path $root "dist\PrivacyBrowser-$version-win64"
if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Path $stage -Force | Out-Null

"Staging into $stage"
Copy-Item $binary $stage

# windeployqt brings the Qt runtime, the QML modules and the Qt WebEngine
# process and resources.
& $windeployqt `
    --release `
    --qmldir (Join-Path $root 'ui') `
    --no-translations `
    --no-system-d3d-compiler `
    (Join-Path $stage 'PrivacyBrowser.exe')
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed" }

# Documentation that has to travel with a build.
foreach ($doc in @('README.md', 'docs\privacy-policy.md', 'docs\threat-model.md')) {
    $source = Join-Path $root $doc
    if (Test-Path $source) {
        Copy-Item $source (Join-Path $stage (Split-Path $doc -Leaf))
    }
}

$licenseFolder = Join-Path $stage 'licenses'
New-Item -ItemType Directory -Path $licenseFolder -Force | Out-Null
$licenseNote = @'
This package includes Qt and Qt WebEngine (which embeds Chromium).

Qt is used under the LGPL v3. Qt WebEngine additionally carries Chromium's own
licences. The licence texts are distributed with the Qt installation used to
build this package; copy them here before publishing a release, and see
docs/licensing.md for the obligations that come with them.

This project's own licence has not been chosen yet - see docs/licensing.md.
'@
Set-Content -Path (Join-Path $licenseFolder 'README.txt') -Value $licenseNote -Encoding utf8

$zip = "$stage.zip"
if (Test-Path $zip) { Remove-Item -Force $zip }
Compress-Archive -Path (Join-Path $stage '*') -DestinationPath $zip

""
"Portable layout: $stage"
"Portable archive: $zip"
"Size: {0:N1} MB" -f ((Get-Item $zip).Length / 1MB)
""
"Before publishing:"
"  1. Copy the Qt and Chromium licence texts into licenses\."
"  2. Choose this project's licence (docs/licensing.md) and add LICENSE."
"  3. Sign PrivacyBrowser.exe and the installer (docs/release.md)."
"  4. Run scripts\verify-privacy.ps1 against the packaged binary."
