@echo off
SET ENVSCRIPT="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
SET SETUP=call %ENVSCRIPT%
SET COMPILE=cl.exe /nologo /Ob0 /O2 ./main.c /Fe"desktop-background.exe" advapi32.lib user32.lib
SET EXECUTE1=desktop-background.exe --help
SET EXECUTE2=desktop-background.exe --get
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
echo.
echo The compilation was successful
echo.
%EXECUTE1%
IF %ERRORLEVEL% NEQ 0 (
  echo.
  echo Execute with help 1 failed 
  echo.
  pause
  goto loop
)
echo.
echo The execution with the help argument was successful
echo.
%EXECUTE2%
IF %ERRORLEVEL% NEQ 0 (
  echo.
  echo Execute with get 2 failed 
  echo.
  pause
  goto loop
)
echo.
echo The execution with the get argument was successful
echo.
echo Execution finished
echo.
pause
goto loop