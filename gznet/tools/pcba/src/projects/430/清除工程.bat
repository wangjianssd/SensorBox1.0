::@echo off
rem ��������...
rem ɾ���ļ�
for %%b in ("%cd%") do cd /d %%b&for /r %%c in ("Debug","settings","Release") do if exist %%c rmdir /s/q  "%%c"
for /f "delims=" %%i in ('dir /b /a-d /s "*.sfr","*.dep","*.tmp","path.txt","*.tmp.c","*.ewp.orig","*.h.bak","*.c.bak","*.orig"') do del /q "%%i"
rem ɾ�����
pause
