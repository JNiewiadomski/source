BLACK="[30m"
RED="[31m"
GREEN="[32m"
BROWN="[33m"
BLUE="[34m"
PURPLE="[35m"
CYAN="[36m"
LIGHT_GRAY="[37m"
DARK_GRAY="[1;30m"
LIGHT_RED="[1;31m"
LIGHT_GREEN="[1;32m"
YELLOW="[1;33m"
LIGHT_BLUE="[1;34m"
LIGHT_PURPLE="[1;35m"
LIGHT_CYAN="[1;36m"
WHITE="[1;37m"
NO_COLOR="[0m"

esc() {
    echo "\e${1}"
}

bash_esc() {
    echo "\[$(esc "${1}")\]"
}

PS1="$(bash_esc "${BLUE}"){ \W } $(bash_esc "${GREEN}")$(git rev-parse --abbrev-ref HEAD 2> /dev/null || echo '\u@\h') $(bash_esc "${RED}")\$»$(bash_esc "${NO_COLOR}") "

PROMPT_COMMAND="RETURN_CODE=\${?} ; if [ \${RETURN_CODE} -ne 0 ] ; then echo -en \"$(esc "${RED}")[\${RETURN_CODE}]$(esc "${NO_COLOR}")\" ; else echo -n \"\" ; fi"

unset BLACK RED GREEN BROWN BLUE PURPLE CYAN LIGHT_GRAY DARK_GRAY LIGHT_RED LIGHT_GREEN YELLOW LIGHT_BLUE LIGHT_PURPLE LIGHT_CYAN WHITE NO_COLOR

unset -f esc bash_esc

