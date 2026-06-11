# EdgeGate local dev (no Docker)
# Starts backend (SQLite) + dashboard

$Root = Split-Path -Parent $PSScriptRoot
$Backend = Join-Path $Root "apps\backend"
$Dashboard = Join-Path $Root "apps\dashboard"

Write-Host "=== EdgeGate Local Dev ===" -ForegroundColor Cyan
Write-Host "Backend:  http://localhost:8000"
Write-Host "Dashboard: http://localhost:3000"
Write-Host "API docs: http://localhost:8000/docs"
Write-Host ""

# Backend
Start-Process powershell -ArgumentList @(
    "-NoExit", "-Command",
    "cd '$Backend'; `$env:PYTHONPATH='.'; python -m uvicorn app.main:app --reload --host 0.0.0.0 --port 8000"
) -WindowStyle Normal

Start-Sleep -Seconds 3

# Dashboard
Start-Process powershell -ArgumentList @(
    "-NoExit", "-Command",
    "cd '$Dashboard'; npm run dev"
) -WindowStyle Normal

Write-Host "Started backend and dashboard in separate windows." -ForegroundColor Green
