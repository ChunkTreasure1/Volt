@echo off

where /q python
IF ERRORLEVEL 1 (    
	where /q py
	IF ERRORLEVEL 1 (	
		ECHO Python is missing.
		exit /b 1)
	call py data/Setup.py
	exit /b 1
)

call python data/Setup.py