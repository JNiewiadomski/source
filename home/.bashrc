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
    #
    # Terminal prompt not wrapping correctly
    # https://unix.stackexchange.com/questions/105958/terminal-prompt-not-wrapping-correctly/447520#447520

    local -r BLACK="\001$(tput setaf 0)\002"
    local -r RED="\001$(tput setaf 1)\002"
    local -r GREEN="\001$(tput setaf 2)\002"
    local -r YELLOW="\001$(tput setaf 3)\002"
    local -r BLUE="\001$(tput setaf 4)\002"
    local -r MAGENTA="\001$(tput setaf 5)\002"
    local -r CYAN="\001$(tput setaf 6)\002"
    local -r WHITE="\001$(tput setaf 7)\002"
    local -r DEFAULT_COLOR="\001$(tput sgr0)\001"

    PS1="${GREEN}\$(git rev-parse --abbrev-ref HEAD 2> /dev/null || echo '\u@\h')${DEFAULT_COLOR} ${YELLOW}\w${DEFAULT_COLOR}\n\$ "

    PROMPT_COMMAND="RETURN_CODE=\${?} ; if [ \${RETURN_CODE} -ne 0 ] ; then echo -en \"${RED}[\${RETURN_CODE}]${DEFAULT_COLOR}\" ; else echo -n \"\" ; fi"

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

    # Only set PAGER if we have access to `most`.
    if which most 2> /dev/null ; then
        export PAGER=most
    fi
}

customize_prompt
define_environment_variables
