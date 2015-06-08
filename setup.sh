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

    # If file is symbolic link we're done.
    if [ -h ${DST} ];
    then
        ;

    # File already exists, but not a symbolic link.
    elif [ -f ${DST} ];
    then
        diff --brief ${DST} ${SRC};

        # File is different in home directory.
        if [ $? -eq 1 ];
        then
            diff ${DST} ${SRC};
            echo "Remove original and create symbolic link (y/n)?";
            read YN;
            if test "${YN}" = "y";
            then
                rm ${DST};
                ln -s ${DIR}/${SRC} ${DST};
            fi

        # File is the same. Offer to remove and create symbolic link.
        else
            echo "No difference in '${DST}' and '${SRC}'. Removing original and creating symbolic link.";
            rm ${DST};
            ln -s ${DIR}/${SRC} ${DST};
        fi

    # File does not exist.
    else
        echo "File '${DST}' does not exist. Creating symbolic link.";
        ln -s ${DIR}/${SRC} ${DST};
    fi
done

###################################################################################################
#
# Create directories used by VIM as specified in .vimrc file.
#
###################################################################################################
mkdir -p ~/.vim/backup
mkdir -p ~/.vim/swap
mkdir -p ~/.vim/undo

