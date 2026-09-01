# Reports whether this machine has the toolchain Privacy Browser needs.
# Read-only: it inspects the system and installs nothing.

$ErrorActionPreference = 'Stop'

function Report($name, $found, $detail) {
    $mark = if ($found) { 'OK  ' } else { 'MISS' }
    "{0} {1,-22} {2}" -f $mark, $name, $detail
}

"Privacy Browser - development environment check"
""

# Visual Studio / MSVC. Qt 6 needs the MSVC 2019 (v142) toolset or newer.
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (Test-Path $vswhere) {
    $vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -format json | ConvertFrom-Json
    if ($vs) {
        $v = $vs[0].installationVersion
        Report 'MSVC C++ toolset' ([version]$v -ge [version]'16.0') "$($vs[0].displayName) ($v)"
    } else {
        Report 'MSVC C++ toolset' $false 'Visual Studio found, but no C++ workload'
    }
} else {
    Report 'MSVC C++ toolset' $false 'Visual Studio Installer not found'
}

foreach ($tool in @(
    @{ Name = 'CMake >= 3.21'; Cmd = 'cmake'; Args = @('--version') },
    @{ Name = 'Ninja';         Cmd = 'ninja'; Args = @('--version') },
    @{ Name = 'Git';           Cmd = 'git';   Args = @('--version') },
    @{ Name = 'Python 3';      Cmd = 'python'; Args = @('--version') }
)) {
    $cmd = Get-Command $tool.Cmd -ErrorAction SilentlyContinue
    if ($cmd) {
        $version = (& $tool.Cmd @($tool.Args) | Select-Object -First 1)
        Report $tool.Name $true $version
    } else {
        Report $tool.Name $false 'not on PATH'
    }
}

# Qt 6. QT_ROOT_DIR is what the CMake presets pass as CMAKE_PREFIX_PATH.
if ($env:QT_ROOT_DIR -and (Test-Path $env:QT_ROOT_DIR)) {
    Report 'Qt 6 (QT_ROOT_DIR)' $true $env:QT_ROOT_DIR
    $webengine = Join-Path $env:QT_ROOT_DIR 'lib\cmake\Qt6WebEngineQuick'
    Report 'Qt WebEngine module' (Test-Path $webengine) $webengine
} else {
    Report 'Qt 6 (QT_ROOT_DIR)' $false 'QT_ROOT_DIR is not set - see docs/build-windows.md'
    Report 'Qt WebEngine module' $false 'unknown until Qt is located'
}

""
"See docs/build-windows.md for what to install and how to build."
