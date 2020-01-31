@echo off
if %1. == . goto :syntax
erase %1
goto :exit

:syntax
echo Syntax: %0 path
goto :exit

:exit
