@echo off
set menu=$%number%.bat

:loop
menu$ %1

if errorlevel 7 goto :notwrite
if errorlevel 6 goto :noitems
if errorlevel 5 goto :syntax
if errorlevel 4 goto :notfound
if errorlevel 3 goto :baddos
if errorlevel 2 goto :once
if errorlevel 1 goto :do
goto :cancel

:do
call %menu%
goto :loop

:once
%menu%

:notwrite
echo MENU unable to open file for writing.
goto :exit

:noitems
echo MENU data file has no items listed.
goto :exit

:syntax
echo MENU encountered an invalid directive.
goto :exit

:notfound
echo MENU data file could not be found.
goto :exit

:baddos
echo MENU requires DOS v2.00 or greater.
goto :exit

:cancel
if exist %menu% erase %menu%.bat

:exit
set menu=
echo.
