#!/bin/bash

###################################################################################################
#
# Creates a soft link to all files in the home/ directory to the ~ directory.
#
###################################################################################################

HOME_FILES="`dirname ~/${0}`/home";
echo Creating soft links to all files in the ${HOME_FILES} directory to the ~ directory.

for SRC in `find ${HOME_FILES} -type f \( ! -name "*.swp" \)`
do
    DST=~/`basename ${SRC}`;

    # Remove existing symbolic links.
    if [ -h ${DST} ];
    then
        unlink ${DST};
    fi

    # Remove existing destination files.
    if [ -f ${DST} ];
    then
        rm -f ${DST};
    fi

    # Create symbolic link to file.
    ln -s ${SRC} ${DST};
done

###################################################################################################
#
# Create directories used by VIM as specified in .vimrc file.
#
###################################################################################################
mkdir -p ~/.vim/backup
mkdir -p ~/.vim/swap
mkdir -p ~/.vim/undo
