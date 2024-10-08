@echo off

REM Check if Python is installed
where /q python
IF ERRORLEVEL 1 (
	where /q py
	IF ERRORLEVEL 1 (
		ECHO Python is missing.
		exit /b 1
	)
	call py Setup.py
	exit /b 1
)

call python Scripts/Setup.py
