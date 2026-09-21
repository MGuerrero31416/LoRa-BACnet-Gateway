$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$wslRoot = (& wsl --cd $repoRoot pwd).Trim()

if (-not $wslRoot) {
    throw "Unable to resolve the repository path in WSL."
}

& wsl bash -lc "command -v gcc >/dev/null 2>&1"
if ($LASTEXITCODE -ne 0) {
    throw "WSL gcc is required to run the host sanity check."
}

$source = "$wslRoot/main/app/lora_packet.c"
$test = "$wslRoot/tests/lora_packet_sanity.c"
$include = "$wslRoot/main/app"

& wsl bash -lc "gcc -std=c11 -Wall -Wextra -Werror '$source' '$test' -I'$include' -o /tmp/lora_packet_sanity && /tmp/lora_packet_sanity"
if ($LASTEXITCODE -ne 0) {
    throw "LoRa packet runtime sanity check failed."
}