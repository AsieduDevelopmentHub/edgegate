# Windows-safe NAPT flash (avoids PlatformIO/esptool UnicodeEncodeError on cp1252 consoles).
$ErrorActionPreference = "Stop"
$env:PYTHONIOENCODING = "utf-8"
$env:PYTHONUTF8 = "1"
Set-Location $PSScriptRoot

Write-Host "Building + uploading esp32-c3-supermini-napt ..." -ForegroundColor Cyan
pio run -e esp32-c3-supermini-napt -t upload *>&1 | Tee-Object -FilePath upload-napt.log
if ($LASTEXITCODE -ne 0) {
    Write-Host "Upload failed. See upload-napt.log" -ForegroundColor Red
    exit $LASTEXITCODE
}
Write-Host "Done. Start monitor with:" -ForegroundColor Green
Write-Host "  pio device monitor -e esp32-c3-supermini-napt"
