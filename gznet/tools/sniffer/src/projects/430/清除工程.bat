@echo off & setlocal EnableDelayedExpansion
rem ��������...
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
::ɾ����Ŀ¼
for /f "delims=" %%j in ('dir !FolderName! /ad /s /b') do rd "%%~sj"
)
pause 
