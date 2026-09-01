# Disk-persistence check (PLAN.md 42).
#
# Runs a built browser, then compares the user's profile locations before and
# after to see what it left behind. This is the check that no amount of reading
# the source can replace: it looks at the disk.
#
#   powershell -ExecutionPolicy Bypass -File scripts\verify-privacy.ps1 `
#       -Executable build\windows-release\bin\PrivacyBrowser.exe -Seconds 20
#
# Browse for a while in the window that opens - visit a few sites, accept a
# cookie banner, download something - then let the timer close it. The more the
# browser has done, the more this check is worth.

param(
    [Parameter(Mandatory = $true)][string]$Executable,
    [int]$Seconds = 20
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path $Executable)) {
    throw "Executable not found: $Executable"
}

# Locations a browser would normally litter.
$watched = @(
    $env:APPDATA,
    $env:LOCALAPPDATA,
    $env:TEMP,
    (Join-Path $env:USERPROFILE 'Documents')
) | Where-Object { $_ -and (Test-Path $_) }

function Get-Snapshot($paths) {
    $snapshot = @{}
    foreach ($path in $paths) {
        Get-ChildItem -Path $path -Recurse -Force -File -ErrorAction SilentlyContinue -Depth 4 |
            ForEach-Object { $snapshot[$_.FullName] = $_.Length }
    }
    return $snapshot
}

"Snapshotting before the run…"
$before = Get-Snapshot $watched
"  {0} files" -f $before.Count

"Starting the browser for $Seconds seconds…"
$process = Start-Process -FilePath $Executable -PassThru
Start-Sleep -Seconds $Seconds

if (-not $process.HasExited) {
    "Closing the browser…"
    $process.CloseMainWindow() | Out-Null
    if (-not $process.WaitForExit(15000)) {
        "The browser did not close on its own; stopping it."
        $process.Kill()
    }
}
$exitCode = $process.ExitCode

# Give the operating system a moment to finish the deletions.
Start-Sleep -Seconds 2

"Snapshotting after the run…"
$after = Get-Snapshot $watched
"  {0} files" -f $after.Count

$new = $after.Keys | Where-Object { -not $before.ContainsKey($_) } | Sort-Object

# Files that are expected, and why.
$expected = @(
    # The settings file, only when the user opted into keeping settings.
    '\PrivacyBrowser\settings.json$',
    # Anything the user deliberately downloaded.
    '\\Downloads\\'
)

$unexpected = @()
foreach ($file in $new) {
    $matched = $false
    foreach ($pattern in $expected) {
        if ($file -match $pattern) { $matched = $true; break }
    }
    if (-not $matched) { $unexpected += $file }
}

""
"Exit code: $exitCode  (2 means the browser reported an incomplete cleanup)"
"New files after the run: {0}" -f $new.Count

$sessionLeftovers = $new | Where-Object { $_ -match 'privacy-browser-' }
if ($sessionLeftovers.Count -gt 0) {
    "FAIL: the session directory was not removed:"
    $sessionLeftovers | ForEach-Object { "  $_" }
}

if ($unexpected.Count -gt 0) {
    "Files left behind that are not on the expected list:"
    $unexpected | ForEach-Object { "  $_" }
} else {
    "No unexpected files were left behind."
}

""
"Note: this checks the user's own locations. It does not prove nothing was"
"written anywhere on the machine - see docs/privacy-policy.md for what the"
"browser does claim."

if ($sessionLeftovers.Count -gt 0 -or $exitCode -eq 2) { exit 1 }
if ($unexpected.Count -gt 0) { exit 2 }
exit 0
