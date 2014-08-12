#!/bin/bash

NO_ARGS=0
E_OPTERROR=85

PARAMLINE=7
PARAMNAME="lp"
ALPHA=0.05

declare -a ESTNAMES

function process_type {
	PARAMNAME=$1
	case $PARAMNAME in
		"lp"		) PARAMLINE=8
				  ESTNAMES=("DA_HE_EDF" "DA_HE_LL" "U_total" "TIME_EXACT" "TIME_DA_HE_EDF" "TIME_DA_HE_LL" "ENERGY") ;;
		"no_lp"		) PARAMLINE=10
				  ESTNAMES=("EDF" "LL" "AUDS" "U_total" "TIME_EXACT" "TIME_AUDS" "TIME_EDF" "TIME_LL" "ENERGY") ;;
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
    *     ) echo "Unimplemented option chosen." && exit $E_OPTERROR;;   # Default.
  esac
done

shift $(($OPTIND - 1))

for c in 1 2; do
	for p in 4 8 16; do
		FILE=result_${PARAMNAME}_${c}_${p}.txt
		for i in `seq $(expr $PARAMLINE - 1)`; do
			MEAN="$(tail -$PARAMLINE $FILE | head -n $i | tail -1 | awk '{ print $2 }')"
			DEVIATION="$(tail -$PARAMLINE $FILE | head -n $i | tail -1 | awk '{ print $3 }')"
			CONF="$(tail -$PARAMLINE $FILE | head -n $i | tail -1 | awk '{ print $4 }')"
			N="$(tail -$PARAMLINE $FILE | head -n $i | tail -1 | awk '{ print $5 }')"
			if [ "$c" == "1" ]; then
				suffix=""
			else
				suffix="s"
			fi
			echo -e "$PARAMNAME\t${ESTNAMES[i - 1]}\t$c cluster$suffix\t$(printf "%02d" $p) processors\t$MEAN\t$DEVIATION\t$CONF\t$ALPHA\t$N"
		done
	done
done
