#!/bin/bash

create_soft_links() {
    local -r HOME_FILES="$(dirname "${0}")/home";

    echo "Creating soft links to files in ${HOME_FILES}"

    local SRC
    local DST

    for SRC in "${HOME_FILES}"/{.[!.],}*
    do
        DST=~/$(basename "${SRC}");

        # Remove existing symbolic links.
        if [ -h "${DST}" ];
        then
            unlink "${DST}";
        fi

        # Remove existing destination files.
        if [ -f "${DST}" ];
        then
            rm -f "${DST}";
        fi

        # Create symbolic link to file.
        echo "${DST} -> ${SRC}"
        ln -s "${SRC}" "${DST}";
    done
}

install_tools() {
    local -r TOOLS="most tig tmux"
    sudo apt-get install -y "${TOOLS}"
}

create_vim_directories() {
    # Create directories used by VIM as specified in .vimrc file.
    mkdir -p ~/.vim/backup
    mkdir -p ~/.vim/swap
    mkdir -p ~/.vim/undo
}

set_login_shell() {
    if [ "${SHELL}" != "/bin/bash" ] ; then
        if sudo where chsh 2> /dev/null ; then
            echo "Setting bash as login shell."
            sudo chsh -s /bin/bash "${USER}"
        else
            echo
            echo "Unable to set bash as login shell for ${USER}: chsh not found."
            echo "Manually edit /etc/passwd."
            return 1
        fi
    fi
}

create_soft_links
install_tools
create_vim_directories
set_login_shell
