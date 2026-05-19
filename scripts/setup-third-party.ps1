# Copies RtAudio 6.0.1 and Steinberg ASIO SDK 2.3.4 into third_party/
param(
    [string]$RtAudioSource = "E:\opensource\RtAudio\rtaudio-6.0.1",
    [string]$AsioSdkSource = "E:\ownCode\ASIO\ASIO-SDK\ASIO-SDK_2.3.4_2025-10-15\ASIOSDK",
    [string]$ProjectRoot = ""
)

$ErrorActionPreference = "Stop"

if (-not $ProjectRoot) {
    $ProjectRoot = Split-Path -Parent $PSScriptRoot
}
if (-not (Test-Path $ProjectRoot)) {
    $ProjectRoot = "E:\ownCode\RtAudio-ASIO"
}

$thirdParty = Join-Path $ProjectRoot "third_party"
$rtaudioDest = Join-Path $thirdParty "rtaudio"
$asioDest = Join-Path $thirdParty "ASIO"

Write-Host "Project root: $ProjectRoot"

function Copy-TreeIfNeeded {
    param([string]$Source, [string]$Dest, [string]$Label)
    if (-not (Test-Path $Source)) {
        throw "Source not found for ${Label}: $Source"
    }
    if (Test-Path $Dest) {
        Write-Host "[skip] $Label already exists at $Dest"
        return
    }
    Write-Host "[copy] $Label -> $Dest"
    New-Item -ItemType Directory -Path (Split-Path $Dest) -Force | Out-Null
    Copy-Item -Path $Source -Destination $Dest -Recurse -Force
}

New-Item -ItemType Directory -Path $thirdParty -Force | Out-Null
Copy-TreeIfNeeded -Source $RtAudioSource -Dest $rtaudioDest -Label "RtAudio"
Copy-TreeIfNeeded -Source $AsioSdkSource -Dest $asioDest -Label "ASIO SDK"

$required = @(
    "$asioDest\common\asio.cpp",
    "$asioDest\host\asiodrivers.cpp",
    "$asioDest\host\pc\asiolist.cpp",
    "$rtaudioDest\RtAudio.cpp",
    "$rtaudioDest\include\iasiothiscallresolver.cpp"
)
foreach ($f in $required) {
    if (-not (Test-Path $f)) {
        throw "Required file missing after setup: $f"
    }
}

Write-Host "Third-party setup complete."
