@echo off

REM Check if Python is installed
where /q python
IF ERRORLEVEL 1 (
	where /q py
	IF ERRORLEVEL 1 (
		ECHO Python is missing.
		exit /b 1
	)
	call py data/Setup.py
	exit /b 1
)

REM Count the number of arguments
set argC=0
for %%x in (%*) do Set /A argC+=1

REM Set projectDir to the passed argument
set "projectDir=%*"

REM Set the VOLT_PROJECT environment variable if there is an argument
IF %argC% gtr 0 (
	setx VOLT_PROJECT "%projectDir%"
) ELSE (
	REM If no argument, use the existing VOLT_PROJECT variable
	IF defined VOLT_PROJECT (
		set "projectDir=%VOLT_PROJECT%"
	)
)

REM Call Python with the appropriate parameters
IF "%projectDir%"=="" (
	call python data/Setup.py
) ELSE (
	call python data/Setup.py -p="%projectDir%"
)
