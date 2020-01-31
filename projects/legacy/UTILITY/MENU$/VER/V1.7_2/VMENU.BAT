@echo off

rem ***********************************************
rem * Check for request to delete temp batch file *
rem ***********************************************
if %2. == . goto :loop
if not %1 == DELETE goto :loop
erase %2
goto :exit

:loop
rem *****************************
rem * Try to make a unique name *
rem *****************************
set menu=$
:name
if not exist %menu%.bat goto :start
if %menu% == $$$$$$$$ goto :badname
set menu=%menu%$
goto :name

rem *********************************************
rem * Display menu and perform selected command *
rem *********************************************
:start
if %1. == . goto :default
if not exist %1.mnu goto :noname
menu$ <%1.mnu >%menu%.bat
goto :check
:default
if not exist main.mnu goto :nodefault
menu$ <main.mnu >%menu%.bat
:check
if errorlevel 3 goto :logout
if errorlevel 2 goto :once
if not errorlevel 1 goto :cancel
cdd /header "%0 DELETE" /footer %menu%.BAT>> %menu%.bat
call %menu%
goto :loop

:once
echo SET MENU=>> %menu%.bat
cdd /header "%0 DELETE" /footer %menu%.BAT>> %menu%.bat
%menu%

:logout
call %menu%
erase %menu%.bat
set menu=
logout

:cancel
if exist %menu%.bat erase %menu%.bat
goto :exit

:badname
cls
echo WARNING
echo.
echo Unable to generate unique menu name ($*.BAT).
echo Press any key to reset and start over.
echo Press Ctrl-Break to terminate execution.
echo (If you are only logged in once, I recommend pressing any key)
echo.
pause
erase $*.bat
goto :loop

:noname
echo File %1.MNU not found.
goto :exit

:nodefault
echo File MAIN.MNU not found.
goto :exit

:exit
set menu=
echo.
