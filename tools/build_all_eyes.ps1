# Build firmware for each eye type. Run after: . $env:IDF_PATH\export.ps1
$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot\..

$eyes = @(
    "defaultEye", "dragonEye", "noScleraEye", "goatEye", "newtEye",
    "terminatorEye", "catEye", "owlEye", "naugaEye", "doeEye"
)
$cfgPath = "main\config.h"
$original = Get-Content $cfgPath -Raw -Encoding UTF8
$failed = @()

function Set-ActiveEye($content, $active) {
    $lines = $content -split "`n"
    $out = foreach ($line in $lines) {
        if ($line -match '^\s*(//)?#include "data/(\w+)\.h"') {
            $name = $Matches[2]
            if ($name -eq $active) { '#include "data/' + $name + '.h"' }
            else { '//#include "data/' + $name + '.h"' }
        } else { $line.TrimEnd("`r") }
    }
    ($out -join "`n") + "`n"
}

foreach ($eye in $eyes) {
    Write-Host -NoNewline "$eye`: "
    Set-Content $cfgPath (Set-ActiveEye $original $eye) -Encoding UTF8 -NoNewline
    idf.py build 2>&1 | Out-Null
    if ($LASTEXITCODE -eq 0) { Write-Host "OK" }
    else { Write-Host "FAIL"; $failed += $eye }
}

Set-Content $cfgPath (Set-ActiveEye $original "terminatorEye") -Encoding UTF8 -NoNewline
Write-Host "Restored terminatorEye"

if ($failed.Count) {
    Write-Host "FAILED: $($failed -join ', ')"
    exit 1
}
Write-Host "All 10 eyes built successfully."
exit 0
