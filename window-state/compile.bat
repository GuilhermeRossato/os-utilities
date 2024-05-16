@echo off
SET ENVSCRIPT="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
SET SETUP=call %ENVSCRIPT%
SET COMPILE=cl.exe /nologo /Ob0 /O2 ./main.c /Fe"window-state.exe" user32.lib
SET EXECUTE1=window-state.exe --help
SET EXECUTE2=window-state.exe --f
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
%EXECUTE1%
IF %ERRORLEVEL% NEQ 0 (
echo.
  echo Execute 1 failed
  echo.
  pause
  goto loop
)
echo.
%EXECUTE2%
IF %ERRORLEVEL% NEQ 0 (
  echo.
  echo Execute 2 failed
  echo.
  pause
  goto loop
)
echo.
echo Execution finished
echo.
pause
goto loop