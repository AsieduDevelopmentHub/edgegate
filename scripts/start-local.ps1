# EdgeGate local dev — use Cursor/VS Code integrated terminals (recommended)
#
# In Cursor: Ctrl+Shift+P → "Tasks: Run Task" → "EdgeGate: Dev Stack"
#
# Or run each task separately:
#   EdgeGate: Backend
#   EdgeGate: Dashboard

Write-Host "=== EdgeGate Local Dev ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Use Cursor integrated terminals (recommended):" -ForegroundColor Yellow
Write-Host "  Ctrl+Shift+P  ->  Tasks: Run Task  ->  EdgeGate: Dev Stack"
Write-Host ""
Write-Host "Services:" -ForegroundColor Green
Write-Host "  Backend:   http://localhost:8000"
Write-Host "  API docs:  http://localhost:8000/docs"
Write-Host "  Dashboard: http://localhost:3000"
Write-Host ""
Write-Host "Manual (current terminal):" -ForegroundColor Yellow
Write-Host '  Backend:   cd apps/backend; $env:PYTHONPATH="."; python -m uvicorn app.main:app --reload --port 8000'
Write-Host "  Dashboard: cd apps/dashboard; npm run dev"
