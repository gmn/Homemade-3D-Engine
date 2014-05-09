#!/bin/bash
#
# intended to replace and be a simplified GNU-make
#

#stop if anything produces error. (Thanks Icculus!)
set -e      

Red='\e[0;31m'
White='\e[0;37m'
Off='\e[0m'

CC=g++
SUFFIX=cpp

LIBS="-lSDL -lSDL_mixer"

#CFLAGS="-I/usr/include -Wall"
#CFLAGS="-I/usr/include -Wall -Wno-missing-braces -Wno-sequence-point"

# for model_obj.cpp code. FIXME: rewrite model_obj
CFLAGS="-I/usr/include -I/usr/local/include -Wall -Wno-missing-braces" 

CFLAGS_DEBUG="-D_DEBUG -DDEBUG -g"
CFLAGS_OPT="-O3 -Wstrict-aliasing=1"
CFLAGS_PROFILE="-pg"

LDFLAGS=-L/usr/lib

MIDIR=.makeinfo

SRC_DIR=src
EXE_DIR=
OBJ_DIR=obj
EXENAME=h3e_demo
SYS=unknown
DEBUG_FILE=fz.log


FILELIST=( \
    main
    logger
    console
    gvar
    hash
    core
    mixer
    misc
    camera
    geometry
    gl_drawing
    gl_shapes
    gl_macros
    game
    primitives
    image
    materials
    mmfile
    model_obj
    gameHandler
)
    

function EE() {
    echo "$@"
    $@
}

function usage_f() {
    echo "usage: ./make.sh [list|clean|all|opt|debug|--help|-h]"
    exit
}

function make_one() {
    if [ ! $PRE_DONE ]; then pre_prep; fi
	EE ${CC} ${CFLAGS} -o "${OBJ_DIR}/$1.o" -c "${SRC_DIR}/$1.${SUFFIX}" 
}

function pre_prep() {
    EE [ -d ${OBJ_DIR} ] || mkdir ${OBJ_DIR}
    PRE_DONE=1
}

function report() {
    echo "-----------------------------------------"
    echo "created binary: ${EXENAME}"
    echo "building for: ${SYS}"
    echo "build type:   $(if [ $OPTIMISE ]; then echo Optimised; else echo Debug; fi)"
    echo CFLAGS=${CFLAGS}
    echo LDFLAGS=${LDFLAGS}
    echo LIBS=${LIBS}
    echo "-----------------------------------------"
}

function make_all() {
    if [ $OPTIMISE ]; then
        CFLAGS="${CFLAGS_OPT} ${CFLAGS}"
    else
        CFLAGS="${CFLAGS_DEBUG} ${CFLAGS}"
    fi
    if [ $PROFILE ]; then
        CFLAGS="${CFLAGS_PROFILE} ${CFLAGS}"
    fi

    if [ ! $PRE_DONE ]; then pre_prep; fi

    # create compilation info directory
    if [ ! -d $MIDIR ]; then mkdir $MIDIR; fi
    

    #
    # OBJECTS
    #
    N=0
    for i in ${FILELIST[*]}; do
        if [ ! -e $MIDIR/$i ]; then
            make_one $i

            # save md5sum of source file
            md5sum ${SRC_DIR}/$i.${SUFFIX} > $MIDIR/$i

        else # md5 exists

            # check md5sum of source file
            MD1=`md5sum ${SRC_DIR}/$i.${SUFFIX}`  
            MD2=`cat $MIDIR/$i`
            if [ "$MD1" != "$MD2" ]; then
                make_one $i

                # save md5sum of source file
                md5sum ${SRC_DIR}/$i.${SUFFIX} > $MIDIR/$i
            fi
        fi

        OBJS[$N]="${OBJ_DIR}/${i}.o"
        let N="$N+1"
    done

    for i in ${OBJS[*]}; do if [ ! -e $i ]; then
        echo -e "${Red}Compilation Errors. Aborting.${Off}"
        exit;
    fi
    done

    rm -f ${EXENAME}

    #
    # LINK and generate EXECUTABLE
    #
    EE ${CC} ${CFLAGS} -o ${EXENAME} ${OBJS[*]} ${LDFLAGS} ${LIBS}

    if [ -x ${EXENAME} ] && [ $OPTIMISE ]; then
        EE strip ${EXENAME}
    fi

    report

    if [ -x ${EXENAME} ]; then
        echo -e "${White}Build Successful${Off}"
    fi
}

function clean_f() {
    rm -rf $MIDIR
    EE rm -rf ${OBJ_DIR}
    EE rm -f ${EXENAME}
    EE [ -e OUT ] && rm OUT
    EE [ -e ${DEBUG_FILE} ] && rm ${DEBUG_FILE}
} 


MATCH=0
try_match_exe() {
    IN=`echo $1 | sed 's/\.cpp//g'`

    local LINES=`echo ${FILELIST[*]} | tr ' ' '\n' | uniq`

    echo "making: "`echo $LINES | tr ' ' '\n' | grep $IN` 
    
    for i in `echo $LINES | tr ' ' '\n' | grep $IN` ; do
        make_one $i
        MATCH=1
    done
}

function list_f() { 
    echo ${FILELIST[*]} | tr ' ' '\n' | sort | uniq 
}

###############################################################################
#
# entry point
#

#guess system
if echo "$(uname -a)" | grep -q -i linux - ; then
    SYS=Linux
elif echo "$(uname -a)" | grep -q -i darwin - ; then
    SYS=Mac
fi

case ${SYS} in
    Linux)
        LIBS="${LIBS} -lGL -lGLU"
        ;;
    Mac)
        # add fink paths
        CFLAGS="${CFLAGS} -D__MAC__"
        LIBS="${LIBS} -lSDLmain -Wl,-framework,Cocoa -framework OpenGL"
        # assumes Terminal.app (FIXME: check)
        Red='\033[0;31m'
        White='\033[0;37m'
        Off='\033[0m'
        ;;
esac



if [ $1 ] ; then
    case $1 in
        list) 
            list_f
            ;;
        clean)
            clean_f
            ;;
        all)
            make_all
            ;;
        prof)
            PROFILE=1
            make_all
            ;;
        opt*)
            OPTIMISE=1
            make_all
            ;;
        debug)
            make_all
            ;;
        --help)
            usage_f
            ;;
        -h)
            usage_f
            ;;
        *)
            try_match_exe $1
            [[ $MATCH -eq 1 ]] || usage_f
            ;;
    esac

    # always handle args specifically
    exit
fi

# no arguments
make_all

