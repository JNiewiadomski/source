@echo off
if not %user%. == . goto :net

rem *************************************
rem * User is NOT logged into a network *
rem *************************************
local
if exist $*.bat erase $*.bat
goto :continue

rem *********************************
rem * User IS logged into a network *
rem *********************************
:net
o:
cd \home\%user%

:continue
vmenu
