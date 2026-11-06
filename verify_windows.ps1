param(
    [string]$BuildDir = "build",
    [string]$Generator = "MinGW Makefiles",
    [int]$Jobs = 2
)
$ErrorActionPreference = "Stop"

$cmake = ".\tools\cmake\bin\cmake.exe"
if (-not (Test-Path $cmake)) {
    $cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
    if (-not $cmakeCommand) { throw "CMake was not found. Install CMake or provide tools\cmake\bin\cmake.exe." }
    $cmake = $cmakeCommand.Source
}

$logDir = ".\verification_results"
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$configureLog = Join-Path $logDir "cmake_configure.log"
$buildLog = Join-Path $logDir "cmake_build.log"
$testLog = Join-Path $logDir "ctest.log"

& $cmake -S . -B $BuildDir -G $Generator 2>&1 | Tee-Object -FilePath $configureLog
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed. See $configureLog" }

& $cmake --build $BuildDir --parallel $Jobs 2>&1 | Tee-Object -FilePath $buildLog
if ($LASTEXITCODE -ne 0) { throw "Build failed. See $buildLog" }

& $cmake -E chdir $BuildDir ctest --output-on-failure 2>&1 | Tee-Object -FilePath $testLog
if ($LASTEXITCODE -ne 0) { throw "CTest failed. See $testLog" }

Write-Host "Build and tests passed. Logs saved in $logDir."
