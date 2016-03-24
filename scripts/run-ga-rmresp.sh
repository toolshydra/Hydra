#!/bin/bash

SCHED_HOME=/opt/hydra/Hydra
SIM_EXACT=$SCHED_HOME/src/solver_mgap_rm_exact
SIM_TIGHT=$SCHED_HOME/src/solver_mgap_edf_tight
SIM_EDF=$SCHED_HOME/src/solver_mgap_edf
SIM_GA=$SCHED_HOME/src/ga/geneticmgap_rm_resp
OUT=$SCHED_HOME/run/exercise_b_10/
export PATH=$PATH:$SCHED_HOME/src

# size of each sample (number of models per config)
SSIZE=30
# maximum time of each execution (10min)
TIME_LIMIT=600

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
	echo "-d TIME_LIMIT($TIME_LIMIT)	- time limit to cplex solver"
	echo "-e TASK_START($TASK_START)	- number of tasks to start"
	echo "-f TASK_STEP($TASK_STEP)		- step of tasks to walk"
	echo "-g TASK_END($TASK_END)		- maximum number of tasks"
	echo "-k UTIL_START($UTIL_START)	- total utilization to start"
	echo "-i UTIL_STEP($UTIL_STEP)	- step of utilization to walk"
	echo "-j UTIL_END($UTIL_END)		- maximum utilization"

}

# command line parsing
while getopts "h?s:d:e:f:g:k:i:j:" opt; do
	case "$opt" in
	h|\?)
		show_help
		exit 0
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
	*)
		echo "invalid option"
		exit -1
		;;
	esac
done

shift $((OPTIND-1))

[ "$1" = "--" ] && shift

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
		files=(file????????.tight)
		nfile=${#files[@]}
		ifile=0
		while [ $ifile -lt $nfile ] ; do
			file=${files[$ifile]}
			file=${file%%.tight}
			ifile=$(($ifile + 1))
		#	udone=""
			# execute ga
		#	if [ -e "$file.ga" ] ; then
		#		if [ "$(cat $file.ga | wc -l)" == "3" ] ; then
		#			if [ "$(head -1 $file.ga)" == "0" ] ; then
		#				udone="yes"
		#			fi
		#		else
		#			if [ $(cat $file.ga | wc -l) -gt 3 ] ; then
		#				udone="yes"
		#			fi
		#		fi
		#	fi
		#	if [ -z "$udone" ] ; then
				TEMPFS="$SIM_GA $file"
				$TEMPFS > $file.ga_rmresp
				#echo "Need to sleep to cool off"
				#sleep 60
		#	fi

		done
		popd >& /dev/null
	done
done

