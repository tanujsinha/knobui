#!/usr/bin/env python3
"""
ESP32-S3 LCD Project Validation Script

This script performs basic validation of the project files
without requiring ESP-IDF to be installed.
"""

import os
import sys
import subprocess
import re

def check_file_exists(filepath, description):
    """Check if a file exists and report the result."""
    if os.path.exists(filepath):
        print(f"✓ {description}: Found")
        return True
    else:
        print(f"✗ {description}: Missing")
        return False

def check_c_syntax(filepath):
    """Basic C syntax checking using regex patterns."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Check for common syntax issues
        issues = []
        
        # Check for unclosed braces
        open_braces = content.count('{')
        close_braces = content.count('}')
        if open_braces != close_braces:
            issues.append(f"Mismatched braces: {open_braces} open, {close_braces} close")
        
        # Check for include statements
        if '#include' not in content:
            issues.append("No include statements found")
        
        # Check for function definitions
        if 'void ' not in content and 'int ' not in content:
            issues.append("No function definitions found")
        
        return issues
    except Exception as e:
        return [f"Error reading file: {e}"]

def validate_project():
    """Validate the ESP32-S3 LCD project structure and files."""
    print("ESP32-S3 Knob Touch LCD Project Validation")
    print("=" * 50)
    
    base_path = os.path.dirname(os.path.abspath(__file__))
    issues = []
    
    # Check project structure
    required_files = [
        ("CMakeLists.txt", "Root CMakeLists.txt"),
        ("main/CMakeLists.txt", "Main component CMakeLists.txt"),
        ("main/main.c", "Main application file"),
        ("main/lcd_driver.c", "LCD driver implementation"),
        ("main/lcd_driver.h", "LCD driver header"),
        ("README.md", "Project README"),
        ("SETUP.md", "Setup instructions"),
        (".vscode/tasks.json", "VS Code tasks"),
    ]
    
    print("\n1. Checking project structure...")
    for filepath, description in required_files:
        full_path = os.path.join(base_path, filepath)
        if not check_file_exists(full_path, description):
            issues.append(f"Missing file: {filepath}")
    
    # Check C source files for basic syntax
    print("\n2. Checking C source syntax...")
    c_files = [
        "main/main.c",
        "main/lcd_driver.c"
    ]
    
    for c_file in c_files:
        full_path = os.path.join(base_path, c_file)
        if os.path.exists(full_path):
            syntax_issues = check_c_syntax(full_path)
            if syntax_issues:
                print(f"✗ {c_file}: Syntax issues found")
                for issue in syntax_issues:
                    print(f"  - {issue}")
                    issues.append(f"{c_file}: {issue}")
            else:
                print(f"✓ {c_file}: Basic syntax OK")
    
    # Check for ESP-IDF availability
    print("\n3. Checking ESP-IDF availability...")
    try:
        result = subprocess.run(['idf.py', '--version'], 
                              capture_output=True, text=True, timeout=10)
        if result.returncode == 0:
            print(f"✓ ESP-IDF found: {result.stdout.strip()}")
        else:
            print("✗ ESP-IDF not found or not working")
            issues.append("ESP-IDF not available")
    except (subprocess.TimeoutExpired, FileNotFoundError):
        print("✗ ESP-IDF not found in PATH")
        issues.append("ESP-IDF not installed or not in PATH")
    
    # Summary
    print("\n" + "=" * 50)
    if issues:
        print(f"❌ Validation failed with {len(issues)} issues:")
        for issue in issues:
            print(f"  - {issue}")
        print("\nRecommendations:")
        if any("ESP-IDF" in issue for issue in issues):
            print("  • Install ESP-IDF using the VS Code extension")
            print("  • Run 'ESP-IDF: Configure ESP-IDF extension' from Command Palette")
        print("  • Review SETUP.md for detailed instructions")
        return False
    else:
        print("✅ All validation checks passed!")
        print("Project is ready for building with ESP-IDF")
        return True

if __name__ == "__main__":
    success = validate_project()
    sys.exit(0 if success else 1)
