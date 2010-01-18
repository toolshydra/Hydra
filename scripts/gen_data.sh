#! /bin/bash

NO_ARGS=0
E_OPTERROR=85


FREQ_FILE=freqs.txt
RANGE_FILE=range.txt
NFREQ=5
NRES=5
SE=/home/evalenti/trees/smartenum
SIMU=$SE/src/pseudosim
SCRIPTS=$SE/scripts
ARGS="-m $NFREQ -p $NRES -f $FREQ_FILE -r $RANGE_FILE"
AKAROA_HOME=/usr/local/akaroa/bin
AKRUN=$AKAROA_HOME/akrun
AKMASTER=$AKAROA_HOME/akmaster
AKSLAVE=$AKAROA_HOME/akslave
NSIMU=8
LOGDIR=$(pwd)/log
DATADIR=$(pwd)/data
GRAPHDIR=$(pwd)/graph
PRUNINGS="- A B C A+B A+C B+C A+B+C"

if [ $# -eq "$NO_ARGS" ]    # Script invoked with no command-line args?
then
  echo "Usage: `basename $0` options (-t)"
  exit $E_OPTERROR          # Exit and explain usage.
                            # Usage: scriptname -options
                            # Note: dash (-) necessary
fi


while getopts "s:e:dn:p:" Option
do
  case $Option in
    s     ) START=$OPTARG ;;
    e     ) END=$OPTARG ;;
    n     ) NSIMU=$OPTARG ;;
    d     ) DRY=true ;;
    p     ) PRUNINGS=$OPTARG ;;
    *     ) echo "Unimplemented option chosen." && exit $E_OPTERROR;;   # Default.
  esac
done

shift $(($OPTIND - 1))

if [ -z "$START" ] ; then
	echo "Specify -s N"
	exit $E_OPTERROR
fi

if [ -z "$END" ] ; then
	echo "Specify -e N"
	exit $E_OPTERROR
fi


function process_results {
	N=$1
	if [ -z "$DRY" ] ; then
		pushd $LOGDIR && $SCRIPTS/process_data.sh -t evaluated -n $N && popd
		pushd $LOGDIR && $SCRIPTS/process_data.sh -t "time" -n $N && popd
		mv $LOGDIR/*dat $DATADIR
		Y=$(head -n 1 $DATADIR/$N.time.dat | awk '{ print $2 }' | cut -d. -f 1)
		if ! [ -z "$(head -n 1 $DATADIR/$N.time.dat | awk '{ print $2 }' | grep +)" ] ; then
			K=$(head -n 1 $DATADIR/$N.time.dat | awk '{ print $2 }' | cut -d+ -f 2)
		fi
		if ! [ -z "$K" ] ; then
			for i in `seq $K` ; do
				Y=$(expr $Y \* 10)
			done
		fi
		Y=$(expr $Y \* 2 )
		pushd $DATADIR && $SCRIPTS/plot_time.sh -y $Y $N.time.dat && popd
		pushd $DATADIR && $SCRIPTS/plot_evaluated.sh $N.evaluated.dat && popd
		mv $DATADIR/*jpg $GRAPHDIR
	fi
}

function do_simul {
	CMD=$1
	date
	echo $CMD
	$CMD
	date
}

# A  -i  --best-initial-limits         Removes combinations by limiting too low frequencies.
# B  -k  --start-drop                  Drop initial useless samples.
# C  -j  --jump-useless                Jump useless detected samples.

function exec_simul {
	NTASK=$1

	SEED=$(cat seed)
	if ! [ -z $SEED ] ; then
		SEED="-r $SEED"
	fi
	echo "Generating simulation batch for $NTASK tasks"
	for P in $PRUNINGS ; do
		O=$(echo $P | sed -e 's/+/ /g')
		O=$(echo $O | sed -e 's/-/ /g')
		O=$(echo $O | sed -e 's/A/-i/g')
		O=$(echo $O | sed -e 's/B/-k/g')
		O=$(echo $O | sed -e 's/C/-j/g')
		echo "Executing type $P ($O)"
		echo "Random: $SEED"
		CMD="$AKRUN $SEED -s -n $NSIMU -- $SIMU $ARGS -n $NTASK $O"
		echo $CMD
		if [ -z "$DRY" ] ; then
			do_simul "$CMD" >& $LOGDIR/$NTASK.$P.log
			SEED=$(grep Random $LOGDIR/$NTASK.$P.log | awk '{ print $2 }')
			if ! [ -z "$SEED" ] ; then
				echo $SEED > seed
				SEED="-r $SEED"
			fi
		fi
	done
}


for d in "$LOGDIR" "$DATADIR" "$GRAPHDIR" ; do
	if ! [ -d "$d" ] ; then
		mkdir -p $d
	fi
done

for n in `seq $START $END` ; do
	exec_simul $n
	process_results $n
done

