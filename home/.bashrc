# ~/.bashrc: executed by bash(1) for non-login shells.
# see /usr/share/doc/bash/examples/startup-files (in the package bash-doc)
# for examples

# If not running interactively, don't do anything
case $- in
    *i*) ;;
      *) return;;
esac

# don't put duplicate lines or lines starting with space in the history.
# See bash(1) for more options
HISTCONTROL=ignoreboth

# append to the history file, don't overwrite it
shopt -s histappend

# for setting history length see HISTSIZE and HISTFILESIZE in bash(1)
HISTSIZE=1000
HISTFILESIZE=2000

# check the window size after each command and, if necessary,
# update the values of LINES and COLUMNS.
shopt -s checkwinsize

# If set, the pattern "**" used in a pathname expansion context will
# match all files and zero or more directories and subdirectories.
#shopt -s globstar

# make less more friendly for non-text input files, see lesspipe(1)
#[ -x /usr/bin/lesspipe ] && eval "$(SHELL=/bin/sh lesspipe)"

# enable color support of ls and also add handy aliases
if [ -x /usr/bin/dircolors ]; then
    test -r ~/.dircolors && eval "$(dircolors -b ~/.dircolors)" || eval "$(dircolors -b)"
    alias ls='ls --color=auto'
    alias grep='grep --color=auto'
    alias fgrep='fgrep --color=auto'
    alias egrep='egrep --color=auto'
fi

# colored GCC warnings and errors
export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

# some more ls aliases
#alias ll='ls -l'
#alias la='ls -A'
#alias l='ls -CF'

# Alias definitions.
# You may want to put all your additions into a separate file like
# ~/.bash_aliases, instead of adding them here directly.
# See /usr/share/doc/bash-doc/examples in the bash-doc package.

if [ -f ~/.bash_aliases ]; then
    source ~/.bash_aliases
fi

# enable programmable completion features (you don't need to enable
# this, if it's already enabled in /etc/bash.bashrc and /etc/profile
# sources /etc/bash.bashrc).
if ! shopt -oq posix; then
  if [ -f /usr/share/bash-completion/bash_completion ]; then
    source /usr/share/bash-completion/bash_completion
  elif [ -f /etc/bash_completion ]; then
    source /etc/bash_completion
  fi
fi

customize_prompt() {
    # Bash/Prompt customization
    # https://wiki.archlinux.org/index.php/Bash/Prompt_customization
    #
    # How to Customize (and Colorize) Your Bash Prompt
    # https://www.howtogeek.com/307701/how-to-customize-and-colorize-your-bash-prompt/

    local -r BLACK="[30m"
    local -r RED="[31m"
    local -r GREEN="[32m"
    local -r BROWN="[33m"
    local -r BLUE="[34m"
    local -r PURPLE="[35m"
    local -r CYAN="[36m"
    local -r LIGHT_GRAY="[37m"
    local -r DARK_GRAY="[1;30m"
    local -r LIGHT_RED="[1;31m"
    local -r LIGHT_GREEN="[1;32m"
    local -r YELLOW="[1;33m"
    local -r LIGHT_BLUE="[1;34m"
    local -r LIGHT_PURPLE="[1;35m"
    local -r LIGHT_CYAN="[1;36m"
    local -r WHITE="[1;37m"
    local -r NO_COLOR="[0m"

    esc() {
        echo "\e${1}"
    }

    bash_esc() {
        echo "\[$(esc "${1}")\]"
    }

    PS1="$(bash_esc "${BLUE}"){$(bash_esc "${NO_COLOR}") \W $(bash_esc "${BLUE}")} $(bash_esc "${GREEN}")\$(git rev-parse --abbrev-ref HEAD 2> /dev/null || echo '\u@\h') $(bash_esc "${RED}")\$»$(bash_esc "${NO_COLOR}") "

    PROMPT_COMMAND="RETURN_CODE=\${?} ; if [ \${RETURN_CODE} -ne 0 ] ; then echo -en \"$(esc "${RED}")[\${RETURN_CODE}]$(esc "${NO_COLOR}")\" ; else echo -n \"\" ; fi"

    unset -f esc bash_esc
}

define_environment_variables() {
    # Environment variables
    # https://wiki.archlinux.org/index.php/Environment_variables
    #
    # Default programs
    #
    #   EDITOR  Contains the command to run the lightweight program used for editing files,
    #           e.g., /usr/bin/nano.
    #
    #   VISUAL  Contains command to run the full-fledged editor that is used for more demanding
    #           tasks, such as editing mail (e.g., vi, vim, emacs etc).
    #
    #   PAGER   Contains command to run the program used to list the contents of files,
    #           e.g., /bin/less.
    #
    export EDITOR=vim
    export VISUAL=${EDITOR}
    export PAGER=most
}

customize_prompt
define_environment_variables
