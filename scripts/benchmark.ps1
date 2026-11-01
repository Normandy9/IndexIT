param(
    [string]$Exe = ".\build\indexit.exe",
    [string]$DataRoot = ".\data",
    [string]$OutputDir = ".\benchmark_results",
    [ValidateRange(1, 10000)][int]$Runs = 100
)

$ErrorActionPreference = "Stop"
$exePath = (Resolve-Path $Exe -ErrorAction Stop).Path
$dataRootPath = (Resolve-Path $DataRoot -ErrorAction Stop).Path
New-Item -ItemType Directory -Force -Path $OutputDir | Out-Null
$outputPath = (Resolve-Path $OutputDir).Path

$datasets = @("small_set", "medium_set", "large_set")
$queries = @("computer", "system", "windows", "information retrieval")
$raw = New-Object System.Collections.Generic.List[object]
$summary = New-Object System.Collections.Generic.List[object]
$datasetInfo = New-Object System.Collections.Generic.List[object]

# Windows command-line quoting: escape backslashes before quotes and at the end.
function Quote-WindowsArgument([string]$Value) {
    if ($Value.Length -gt 0 -and $Value -notmatch '[\s"]') { return $Value }
    $builder = New-Object System.Text.StringBuilder
    [void]$builder.Append('"')
    $slashes = 0
    foreach ($ch in $Value.ToCharArray()) {
        if ($ch -eq '\') { $slashes++; continue }
        if ($ch -eq '"') {
            [void]$builder.Append(('\' * (2 * $slashes + 1)))
            [void]$builder.Append('"')
            $slashes = 0
            continue
        }
        if ($slashes -gt 0) { [void]$builder.Append(('\' * $slashes)); $slashes = 0 }
        [void]$builder.Append($ch)
    }
    if ($slashes -gt 0) { [void]$builder.Append(('\' * (2 * $slashes))) }
    [void]$builder.Append('"')
    return $builder.ToString()
}

function Invoke-TimedIndexIt([string[]]$Arguments, [string]$LogPath) {
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $exePath
    $psi.Arguments = (($Arguments | ForEach-Object { Quote-WindowsArgument $_ }) -join ' ')
    $psi.WorkingDirectory = (Get-Location).Path
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $psi
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    if (-not $proc.Start()) { throw "Could not start $exePath" }
    # Read both streams asynchronously to avoid a full pipe blocking the child process.
    $stdoutTask = $proc.StandardOutput.ReadToEndAsync()
    $stderrTask = $proc.StandardError.ReadToEndAsync()
    $proc.WaitForExit()
    $stdout = $stdoutTask.Result
    $stderr = $stderrTask.Result
    $sw.Stop()
    ($stdout + "`r`n" + $stderr) | Set-Content -Encoding UTF8 $LogPath
    return [pscustomobject]@{ Milliseconds = [math]::Round($sw.Elapsed.TotalMilliseconds, 3); ExitCode = $proc.ExitCode }
}

# Capture enough machine/build context to make later comparisons interpretable.
$os = Get-CimInstance Win32_OperatingSystem
$cpu = Get-CimInstance Win32_Processor | Select-Object -First 1
$exeHash = (Get-FileHash -Algorithm SHA256 $exePath).Hash
$metadata = @(
    [pscustomobject]@{ key = 'timestamp_local'; value = (Get-Date).ToString('o') },
    [pscustomobject]@{ key = 'computer_name'; value = $env:COMPUTERNAME },
    [pscustomobject]@{ key = 'os'; value = $os.Caption + ' ' + $os.Version },
    [pscustomobject]@{ key = 'cpu'; value = $cpu.Name },
    [pscustomobject]@{ key = 'ram_gb'; value = [math]::Round($os.TotalVisibleMemorySize / 1MB, 2) },
    [pscustomobject]@{ key = 'powershell'; value = $PSVersionTable.PSVersion.ToString() },
    [pscustomobject]@{ key = 'executable'; value = $exePath },
    [pscustomobject]@{ key = 'executable_sha256'; value = $exeHash },
    [pscustomobject]@{ key = 'runs_per_query'; value = $Runs },
    [pscustomobject]@{ key = 'timing_scope'; value = 'end-to-end process wall time; includes process startup and CLI overhead' }
)
$metadata | Export-Csv -NoTypeInformation -Encoding UTF8 (Join-Path $outputPath 'benchmark_environment.csv')

foreach ($datasetName in $datasets) {
    $datasetPath = Join-Path $dataRootPath $datasetName
    if (-not (Test-Path $datasetPath -PathType Container)) {
        Write-Warning "Skipping missing dataset: $datasetPath"
        continue
    }
    $files = @(Get-ChildItem -LiteralPath $datasetPath -File -Recurse)
    $bytes = ($files | Measure-Object -Property Length -Sum).Sum
    $datasetInfo.Add([pscustomobject]@{ dataset = $datasetName; file_count = $files.Count; total_bytes = [long]$bytes })

    # Use an isolated cache for every dataset and remove stale benchmark cache first.
    $cachePath = Join-Path (Join-Path $outputPath 'cache') $datasetName
    if (Test-Path $cachePath) { Remove-Item -LiteralPath $cachePath -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $cachePath | Out-Null

    $indexLog = Join-Path $outputPath ("index_" + $datasetName + '.log')
    $indexResult = Invoke-TimedIndexIt @('index', $datasetPath, '--cache-dir', $cachePath) $indexLog
    $raw.Add([pscustomobject]@{ dataset = $datasetName; phase = 'index'; run = 1; query = ''; elapsed_ms = $indexResult.Milliseconds; exit_code = $indexResult.ExitCode })
    if ($indexResult.ExitCode -ne 0) {
        Write-Warning "Indexing failed for $datasetName. See $indexLog"
        continue
    }

    foreach ($query in $queries) {
        $safeQuery = ($query -replace '[^A-Za-z0-9]+', '_').Trim('_')
        $warmLog = Join-Path $outputPath ("warmup_" + $datasetName + '_' + $safeQuery + '.log')
        $warm = Invoke-TimedIndexIt @('search', $datasetPath, $query, '--cache-dir', $cachePath) $warmLog
        if ($warm.ExitCode -ne 0) { Write-Warning "Warm-up search failed: $datasetName / $query. See $warmLog" }

        $samples = New-Object System.Collections.Generic.List[double]
        for ($run = 1; $run -le $Runs; $run++) {
            $log = Join-Path $outputPath ("search_" + $datasetName + '_' + $safeQuery + '_' + $run + '.log')
            $result = Invoke-TimedIndexIt @('search', $datasetPath, $query, '--cache-dir', $cachePath) $log
            $raw.Add([pscustomobject]@{ dataset = $datasetName; phase = 'search'; run = $run; query = $query; elapsed_ms = $result.Milliseconds; exit_code = $result.ExitCode })
            if ($result.ExitCode -eq 0) { $samples.Add($result.Milliseconds) }
            else { Write-Warning "Search failed: $datasetName / '$query' / run $run (exit $($result.ExitCode)); see $log" }
        }

        if ($samples.Count -gt 0) {
            $sorted = @($samples | Sort-Object)
            $middle = [int][math]::Floor($sorted.Count / 2)
            $median = if ($sorted.Count % 2) { $sorted[$middle] } else { ($sorted[$middle - 1] + $sorted[$middle]) / 2 }
            $p95Index = [math]::Min($sorted.Count - 1, [int][math]::Ceiling(0.95 * $sorted.Count) - 1)
            $stats = $samples | Measure-Object -Average -Minimum -Maximum
            $summary.Add([pscustomobject]@{
                dataset = $datasetName; query = $query; successful_runs = $samples.Count; failed_runs = ($Runs - $samples.Count)
                mean_ms = [math]::Round($stats.Average, 3); median_ms = [math]::Round($median, 3)
                p95_ms = [math]::Round($sorted[$p95Index], 3); min_ms = [math]::Round($stats.Minimum, 3); max_ms = [math]::Round($stats.Maximum, 3)
            })
        }
    }
}

$datasetInfo | Export-Csv -NoTypeInformation -Encoding UTF8 (Join-Path $outputPath 'benchmark_datasets.csv')
$raw | Export-Csv -NoTypeInformation -Encoding UTF8 (Join-Path $outputPath 'benchmark_raw.csv')
$summary | Export-Csv -NoTypeInformation -Encoding UTF8 (Join-Path $outputPath 'benchmark_summary.csv')
if ($datasetInfo.Count -eq 0) { throw "No benchmark datasets found under $dataRootPath" }
Write-Host "Benchmark complete. CSV files and per-run logs are in $outputPath."
Write-Host "Review failed_runs and exit_code before interpreting results. Timings include process startup and CLI overhead."
Write-Host "No Windows Search comparison is performed by this script."
