#!/bin/bash

SCHED_HOME=/opt/hydra/Hydra
SIM_EXACT=$SCHED_HOME/src/solver_mgap_rm_exact
SIM_TIGHT=$SCHED_HOME/src/solver_mgap_edf_tight
SIM_EDF=$SCHED_HOME/src/solver_mgap_edf
OUT=$SCHED_HOME/run/20160104_fixed/exac
export PATH=$PATH:$SCHED_HOME/src

# size of each sample (number of models per config)
SSIZE=30
# starting point
SSTART=0
# maximum time of each execution (10min)
TIME_LIMIT=600
NFILE=0

# 5 tasks to 30 tasks (every 5)
TASK_START=5
TASK_STEP=5
TASK_END=30

# 10% to 90%  (every 10%)
UTIL_START=10
UTIL_STEP=10
UTIL_END=90

show_help()
{
	echo "$0 <option>"
	echo "-s SSIZE($SSIZE)		- number of samples per configuration"
	echo "-q SSIZE($SSTART)		- starting model"
	echo "-d TIME_LIMIT($TIME_LIMIT)	- time limit to cplex solver (0 = nolim)"
	echo "-e TASK_START($TASK_START)	- number of tasks to start"
	echo "-f TASK_STEP($TASK_STEP)		- step of tasks to walk"
	echo "-g TASK_END($TASK_END)		- maximum number of tasks"
	echo "-k UTIL_START($UTIL_START)	- total utilization to start"
	echo "-i UTIL_STEP($UTIL_STEP)	- step of utilization to walk"
	echo "-j UTIL_END($UTIL_END)		- maximum utilization"
	echo "-l NFILE($NFILE)			- maximum files to process"

}

# command line parsing
while getopts "h?s:d:e:f:g:k:i:j:l:q:" opt; do
	case "$opt" in
	h|\?)
		show_help
		exit 0
		;;
	q)
		SSTART=$OPTARG
		;;
	s)
		SSIZE=$OPTARG
		;;
	d)
		TIME_LIMIT=$OPTARG
		;;
	e)
		TASK_START=$OPTARG
		;;
	f)
		TASK_STEP=$OPTARG
		;;
	g)
		TASK_END=$OPTARG
		;;
	k)
		UTIL_START=$OPTARG
		;;
	i)
		UTIL_STEP=$OPTARG
		;;
	j)
		UTIL_END=$OPTARG
		;;
	l)
		NFILE=$OPTARG
		;;
	*)
		echo "invalid option"
		exit -1
		;;
	esac
done

shift $((OPTIND-1))

[ "$1" = "--" ] && shift

if [ "$TIME_LIMIT" == "0" ]; then
	TIME_PAR=""
else
	TIME_PAR="-d $TIME_LIMIT"
fi

TOTAL=$(echo "$SSIZE*$(seq -w $TASK_START $TASK_STEP $TASK_END | wc -l) * $(seq -w $UTIL_START $UTIL_STEP $UTIL_END | wc -l)" | bc)

init=$(date)
P=0
for NTASKS in  `seq -w $TASK_START $TASK_STEP $TASK_END` ; do
	for ut in  `seq $UTIL_START $UTIL_STEP $UTIL_END` ; do
		if [ "$NTASKS" == "5" ] ; then
			NTASKS="05"
		fi
		DIR=$OUT/$ut/$NTASKS/
		pushd $DIR 
		nsuccess=0
		files=(file????????)
		if [ $NFILE -gt 0 ] ; then
			nfile=$NFILE
		else
			nfile=${#files[@]}
		fi
		ifile=0
		while [ $nsuccess -lt $SSIZE ] && [ $ifile -lt $nfile ] ; do
			file=${files[$(($SSTART + $ifile))]}
			ifile=$(($ifile + 1))
			udone=""
			# execute tight
			if [ -e "$file.tight" ] ; then
				if [ "$(cat $file.tight | wc -l)" == "3" ] ; then
					if [ "$(head -1 $file.tight)" == "0" ] ; then
						udone="yes"
					fi
				else
					if [ $(cat $file.tight | wc -l) -gt 3 ] ; then
						udone="yes"
					fi
				fi
			fi
			if [ -z "$udone" ] ; then
				TEMPFS="$SIM_TIGHT $TIME_PAR -stm $file"
				touch $file.tight
				$TEMPFS  > $file.tight
			fi
			usucc=$(head -1 $file.tight)

			edone=""
			# execute exact
			if [ -e "$file.exact" ] ; then
				if [ "$(cat $file.exact | wc -l)" == "3" ] ; then
					if [ "$(head -1 $file.exact)" == "0" ] ; then
						edone="yes"
					fi
				else
					if [ $(cat $file.exact | wc -l) -gt 3 ] ; then
						edone="yes"
					fi
				fi
			fi
			if [ -z "$edone" ] ; then
				TEMPFS="$SIM_EXACT $TIME_PAR -stm $file"
			#	$TEMPFS > $file.exact
			fi
			#esucc=$(head -1 $file.exact)

			edfdone=""
			# execute edf
			if [ -e "$file.edf" ] ; then
				if [ "$(cat $file.edf | wc -l)" == "3" ] ; then
					if [ "$(head -1 $file.edf)" == "0" ] ; then
						edfdone="yes"
					fi
				else
					if [ $(cat $file.edf | wc -l) -gt 3 ] ; then
						edfdone="yes"
					fi
				fi
			fi
			if [ -z "$edfdone" ] ; then
				TEMPFS="$SIM_EDF $TIME_PAR -stm $file"
			#	$TEMPFS > $file.edf
			fi
			#edfsucc=$(head -1 $file.edf)

			if [ "$esucc" == "1" ] && [ "$usucc" == "1" ] && [ "$edfsucc" == "1" ] ; then
				nsuccess=$(($nsuccess + 1))
				P=$(($P + 1))
				PERC=$(echo "scale=2;100*($P/$TOTAL)" | bc)
				echo "Progress: $P of $TOTAL ($PERC %) at $ut/$NTASKS ($init --> $(date))"
			fi
		done
		popd >& /dev/null
	done
done

