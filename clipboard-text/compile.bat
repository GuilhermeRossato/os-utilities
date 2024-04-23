@echo off
SET ENVSCRIPT="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
SET SETUP=call %ENVSCRIPT%
SET COMPILE=cl.exe /nologo /Ob0 /O2 ./main.c /Fe"clipboard-text.exe" user32.lib
SET EXECUTE=clipboard-text.exe --help
%SETUP%
IF %ERRORLEVEL% NEQ 0 (
	echo Warning: Environment setup failed
	pause
)
WHERE cl.exe
IF %ERRORLEVEL% NEQ 0 (
	echo Warning: Could not find the compiler executable
	pause
)
:loop
%COMPILE%
echo.
IF %ERRORLEVEL% NEQ 0 (
	echo Compilation failed
  echo.
  pause
  goto loop
)
%EXECUTE%
echo.
IF %ERRORLEVEL% NEQ 0 (
	echo Execute failed
  echo.
  pause
  goto loop
)
echo Execution finished
echo.
pause
goto loop