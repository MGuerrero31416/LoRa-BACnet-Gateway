param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$IdfArguments
)

$ErrorActionPreference = "Stop"

$ExpectedIdfPath = "C:\esp\v6.0.2\esp-idf"
$ActivationScript = if ($env:ESP_IDF_ACTIVATION_SCRIPT) {
    $env:ESP_IDF_ACTIVATION_SCRIPT
} else {
    Join-Path $ExpectedIdfPath "export.ps1"
}
$IdfPyScript = Join-Path $ExpectedIdfPath "tools\idf.py"

if (-not (Test-Path -LiteralPath $ActivationScript -PathType Leaf)) {
    throw "ESP-IDF activation script not found: $ActivationScript"
}

if (-not (Test-Path $IdfPyScript)) {
    throw "Required ESP-IDF tool not found: $IdfPyScript"
}

Write-Host "Activating required ESP-IDF: $ExpectedIdfPath"

$global:LASTEXITCODE = 0
& $ActivationScript

# Installation Manager profiles can leave a nonzero exit code from shell completion.
if (-not $env:ESP_IDF_ACTIVATION_SCRIPT -and $LASTEXITCODE -ne 0) {
    throw "ESP-IDF 6.0.2 environment activation failed."
}

$ResolvedExpectedPath = (Resolve-Path $ExpectedIdfPath).Path.TrimEnd("\")
$ResolvedActualPath = (Resolve-Path $env:IDF_PATH).Path.TrimEnd("\")

if ($ResolvedActualPath -ne $ResolvedExpectedPath) {
    throw @"
Incorrect ESP-IDF environment.

Required: $ResolvedExpectedPath
Active:   $ResolvedActualPath

Build aborted before CMake or Ninja could run.
"@
}

$IdfVersion = (& python $IdfPyScript --version 2>&1 | Out-String).Trim()

if ($LASTEXITCODE -ne 0) {
    throw "Unable to determine the active ESP-IDF version."
}

if ($IdfVersion -notmatch "6\.0\.2") {
    throw @"
Incorrect ESP-IDF version.

Required: ESP-IDF 6.0.2
Detected: $IdfVersion

Build aborted.
"@
}

if (-not $IdfArguments -or $IdfArguments.Count -eq 0) {
    $IdfArguments = @("build")
}

Write-Host ""
Write-Host "Environment verified:"
Write-Host "  IDF_PATH: $env:IDF_PATH"
Write-Host "  Version:  $IdfVersion"
Write-Host "  Command:  idf.py $($IdfArguments -join ' ')"
Write-Host ""

& python $IdfPyScript @IdfArguments
exit $LASTEXITCODE
