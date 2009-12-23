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


while getopts "t:n:" Option
do
  case $Option in
    t     ) process_type "$OPTARG" ;;
    n     ) N=$OPTARG ;;
    *     ) echo "Unimplemented option chosen." && exit $E_OPTERROR;;   # Default.
  esac
done

shift $(($OPTIND - 1))

if [ -z "$N" ] ; then
	echo "Specify -n N"
	exit $E_OPTERROR
fi

for P in - A B C A+B A+C B+C A+B+C ; do
	i=$N.$P.log
	CASE="$(echo $i | cut -d. -f 2 -)"
	MEAN="$(tail -$PARAMLINE $i | head -n 1 | awk '{ print $2 }')"
	DEVIATION="$(tail -$PARAMLINE $i | head -n 1 | awk '{ print $3 }')"
	echo -e "$CASE\t$MEAN\t$DEVIATION"
done > $N.$PARAMNAME.dat
