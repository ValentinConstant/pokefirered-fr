@echo off
REM Lanceur de l'editeur de textes pokefirered (double-cliquez ce fichier)
cd /d "%~dp0"
python edit_text.py
if errorlevel 1 (
  echo.
  echo Python est-il installe et dans le PATH ? Essayez : py edit_text.py
  pause
)
