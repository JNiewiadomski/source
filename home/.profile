ssh_agent() {
    # Working with SSH key passphrases
    # https://help.github.com/en/github/authenticating-to-github/working-with-ssh-key-passphrases

    local -r ENV=~/.ssh/agent.env

    agent_load_env() {
        test -f "${ENV}" && source "${ENV}" >| /dev/null
    }

    agent_start () {
        (umask 077 ; ssh-agent >| "${ENV}")
        source "${ENV}" >| /dev/null
    }

    agent_load_env

    # agent_run_state: 0=agent running w/ key; 1=agent w/o key; 2= agent not running
    AGENT_RUN_STATE=$(ssh-add -l >| /dev/null 2>&1 ; echo $?)

    if [ ! "$SSH_AUTH_SOCK" ] || [ "${AGENT_RUN_STATE}" = 2 ] ; then
        agent_start
        ssh-add
    elif [ "$SSH_AUTH_SOCK" ] && [ "${AGENT_RUN_STATE}" = 1 ] ; then
        ssh-add
    fi
}

run_optional_startup_scripts() {
    if [ -d ~/.profile.d ] ; then
        for SCRIPT in ~/.profile.d/* ; do
            source "${SCRIPT}"
        done
    fi
}

ssh_agent
run_optional_startup_scripts

source ~/.bashrc
