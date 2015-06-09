#!/bin/zsh

###################################################################################################
#
# Setup rc files in rcfiles directory
#   Takes all files from rcfiles directory, and creates a soft link to them from the home
#   directory, with a . prefixed to the file name.
#
###################################################################################################

RCFILES="`dirname ~/${0}`/rcfiles";

for SRC in `find ${RCFILES} -type f \( ! -name "*.swp" \)`
do
    DST=~/.`basename ${SRC}`;

    # If the destination file exists remove it.
    if [ -f ${DST} ];
    then
        rm ${DST};
    fi

    # Create symbolic link to file.
    ln -s ${DIR}/${SRC} ${DST};
done

###################################################################################################
#
# Create directories used by VIM as specified in .vimrc file.
#
###################################################################################################
mkdir -p ~/.vim/backup
mkdir -p ~/.vim/swap
mkdir -p ~/.vim/undo

