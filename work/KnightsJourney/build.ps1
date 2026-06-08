param(
    [string]$Output = "output\KnightsJourney.exe"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path "output")) {
    New-Item -ItemType Directory -Path "output" | Out-Null
}

g++ -std=c++11 `
    -finput-charset=UTF-8 `
    -fexec-charset=GBK `
    -I. main.cpp src/*.cpp `
    -static -static-libgcc -static-libstdc++ `
    -leasyx -lgdi32 -lole32 -lwinmm `
    -o $Output

if ($LASTEXITCODE -ne 0) {
    throw "Build failed with exit code $LASTEXITCODE"
}

Write-Host "Built $Output"
