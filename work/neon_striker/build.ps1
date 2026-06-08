param(
    [string]$Output = "output\NeonStriker.exe"
)

$ErrorActionPreference = "Stop"

Push-Location $PSScriptRoot

if (-not (Test-Path "output")) {
    New-Item -ItemType Directory -Path "output" | Out-Null
}

g++ -std=c++11 `
    -finput-charset=UTF-8 `
    -fexec-charset=GBK `
    main.cpp GameManager.cpp Player.cpp Enemy.cpp Boss.cpp Bullet.cpp Particle.cpp Input.cpp Renderer.cpp Audio.cpp Collision.cpp `
    -static -static-libgcc -static-libstdc++ `
    -leasyx -lgdi32 -lole32 -lwinmm `
    -o $Output

if ($LASTEXITCODE -ne 0) {
    Pop-Location
    throw "Build failed with exit code $LASTEXITCODE"
}

Write-Host "Build success: $Output"

if ($args[0] -eq "-run") {
    Write-Host "Running..."
    & $Output
}

Pop-Location
