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

define sb
    save breakpoints breakpoints.txt
end

define lb
    source ./breakpoints.txt
end

printf "\n------------------------------------------\n"
printf "fc - focus on command window (focus cmd)\n"
printf "fs - focus on src window (focus src)\n"
printf "layout - split window\n"
printf "--------------------------------------------\n\n"

# this will make debugging the XL process more pleasant
handle SIGPIPE nostop noprint pass

# macros for printing Qt nicely
# source ~/unix/kde-qt.gdb

# macros for stl nicely
source ~/unix/stl-views.gdb
