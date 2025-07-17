#!/bin/bash
# Quick update script for ESP32-S3 project
# Usage: ./quick-update.sh "Your commit message"

if [ -z "$1" ]; then
    echo "Usage: ./quick-update.sh \"Your commit message\""
    echo "Example: ./quick-update.sh \"Added new haptic patterns\""
    exit 1
fi

echo "🔄 Updating ESP32-S3 Haptic Counter..."

# Add all changes
git add .

# Commit with timestamp and message
commit_msg="$1 - $(date '+%Y-%m-%d %H:%M')"
git commit -m "$commit_msg"

# Push to GitHub
git push

echo "✅ Successfully updated GitHub repository!"
echo "📝 Commit: $commit_msg"
echo "🔗 Repository: https://github.com/zwood425/swubasedatapad"
