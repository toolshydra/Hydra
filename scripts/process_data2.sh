#!/bin/bash

NO_ARGS=0
E_OPTERROR=85

PARAMLINE=3
PARAMNAME="evaluated"

function process_type {
	PARAMNAME=$1
	case $PARAMNAME in
		"evaluated"	) PARAMLINE=4 ;;
		"time"		) PARAMLINE=3 ;;
		*		) echo "Wrong type $1" && exit $E_OPTERROR ;;
	esac
}

if [ $# -eq "$NO_ARGS" ]    # Script invoked with no command-line args?
then
  echo "Usage: `basename $0` options (-t)"
  exit $E_OPTERROR          # Exit and explain usage.
                            # Usage: scriptname -options
                            # Note: dash (-) necessary
fi


while getopts "t:s:e:" Option
do
  case $Option in
    t     ) process_type "$OPTARG" ;;
    s     ) S=$OPTARG ;;
    e     ) E=$OPTARG ;;
    *     ) echo "Unimplemented option chosen." && exit $E_OPTERROR;;   # Default.
  esac
done

shift $(($OPTIND - 1))

if [ -z "$S" ] ; then
	echo "Specify -s start"
	exit $E_OPTERROR
fi

if [ -z "$E" ] ; then
	echo "Specify -e end"
	exit $E_OPTERROR
fi

for P in - A B C A+B A+C B+C A+B+C ; do
	echo -e -n "$P\t"
	for N in `seq $S $E` ; do
		i=$N.$P.log
		CASE="$(echo $i | cut -d. -f 2 -)"
		MEAN="$(tail -$PARAMLINE $i | head -n 1 | awk '{ print $2 }')"
		DEVIATION="$(tail -$PARAMLINE $i | head -n 1 | awk '{ print $3 }')"
		echo -e -n "$N\t$MEAN\t$DEVIATION\t"
	done
	echo -e -n "\n"
done > $PARAMNAME.dat
