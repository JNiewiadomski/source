set print pretty on
set print object on
set print vtbl on
set print demangle on

# sets the focus to command window
define fc
    focus cmd
end

# sets the focus to src window
define fs
    focus src
end

define lb
    source ./breakpoints.txt
end

define sb
    save breakpoints breakpoints.txt
end

printf "\n------------------------------------------------------\n"
printf "fc - focus on command window (focus cmd)\n"
printf "fs - focus on src window (focus src)\n"
printf "lb - load breakpoints (source ./breakpoints.txt)\n"
printf "sb - save breakpoints (save breakpoints breakpoints.txt)\n"
printf "layout - split window\n"
printf "--------------------------------------------------------\n\n"

# this will make debugging the XL process more pleasant
handle SIGPIPE nostop noprint pass

# make GDB pass the signal straight to the inferior (being debugged) process
handle SIGINT nostop print pass

# macros for printing Qt nicely
# source ~/unix/gdb/kde-qt.gdb

# macros for printing stl nicely
source ~/unix/gdb/stl-views.gdb
