# Quick update script for ESP32-S3 project (PowerShell)
# Usage: .\quick-update.ps1 "Your commit message"

param(
    [Parameter(Mandatory=$true)]
    [string]$CommitMessage
)

Write-Host "🔄 Updating ESP32-S3 Haptic Counter..." -ForegroundColor Cyan

# Add all changes
git add .

# Commit with timestamp and message
$timestamp = Get-Date -Format "yyyy-MM-dd HH:mm"
$fullCommitMsg = "$CommitMessage - $timestamp"
git commit -m $fullCommitMsg

# Push to GitHub
git push

Write-Host "✅ Successfully updated GitHub repository!" -ForegroundColor Green
Write-Host "📝 Commit: $fullCommitMsg" -ForegroundColor Yellow
Write-Host "🔗 Repository: https://github.com/zwood425/swubasedatapad" -ForegroundColor Blue
