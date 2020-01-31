@echo off

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
cdd /header MENUDEL /footer %menu%.BAT>> %menu%.bat
call %menu%
goto :loop

:once
echo SET MENU=>> %menu%.bat
cdd /header MENUDEL /footer %menu%.BAT>> %menu%.bat
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
echo Unable to generate unique menu name.
goto :exit

:noname
echo File %1.MNU not found.
goto :exit

:nodefault
echo File MAIN.MNU not found.
goto :exit

:exit
set menu=
echo.
