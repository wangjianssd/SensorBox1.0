@echo off & setlocal EnableDelayedExpansion
rem ÕýÔÚËÑË÷...
for /f "delims=" %%i in ('dir /a/s/b/on "*.ewd","*.ewp","*.xcl"') do (
set file=%%~pi
set file=!file:/=/!
set FolderName=!file!
echo !FolderName!
for /f "delims=" %%i in ('dir !FolderName! /a-d /b /s') do (
if not %%~xi==.eww (
if not %%~xi==.xcl (
if not %%~xi==.ewp (
if not %%~xi==.ewd (
del /s /a /q "%%~si"
)
)
)
)
)
::É¾³ý¿ÕÄ¿Â¼
for /f "delims=" %%j in ('dir !FolderName! /ad /s /b') do rd "%%~sj"
)
for %%b in ("%cd%") do cd /d %%b&for /r %%c in ("settings") do if exist %%c rmdir /s/q  "%%c"
pause 
