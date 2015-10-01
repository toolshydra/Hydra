#!/bin/bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/eduardo.valentin/lib


AK_HOME=/opt/akaroa2
AKRUN=$AK_HOME/bin/akrun
AK_INSTANCES=4
AK_PARAMS=models/simulation_sample/ak_params.txt

SCHED_HOME=$PWD
SIM=$SCHED_HOME/src/hydra_solver
LOG=$SCHED_HOME/results/utilization_vs_exact_ref_0.95_rm_fix_prio_order

LP=0.45

export PATH=$PATH:$SCHED_HOME/src

mkdir -p $LOG

for t in lp ; do
	if [ "$t" == "lp" ] ; then
		PARAM=""
		NTASKS=1
	else
		PARAM="--compare-no-lp"
		NTASKS=1
	fi
	RANGES=$SCHED_HOME/models/simulation_${t}/modelsimperiodrange.txt
	for c in 1 ; do
		FREQS=$SCHED_HOME/models/simulation_${t}/modelsimfreqs-${c}cluster.txt
		SOLVERS_CONFIG=$SCHED_HOME/models/simulation_${t}/solvers_config.txt
		for p in 1.6 2.8 3.6 ; do
			TEMPFS="$SIM --freq-file=$FREQS --range-file=$RANGES --processor-count=4 --u-total=$p --max-ui=0.5 --task-count=$NTASKS --switch-latency=$LP $PARAM --solvers-file=$SOLVERS_CONFIG"
			echo "$TEMPFS" > $LOG/result_${t}_${c}_${p}.txt
			date >> $LOG/result_${t}_${c}_${p}.txt
			$AKRUN -n $AK_INSTANCES -s -f $AK_PARAMS -- $TEMPFS >> $LOG/result_${t}_${c}_${p}.txt
			date >> $LOG/result_${t}_${c}_${p}.txt
			rm /tmp/file*
			#somehow we are not able to clean this properly
			for i in  `ps aux | grep pseudosim | awk '{print $2}'` ; do kill -9 $i ; done
			for i in  `ps aux | grep solver | awk '{print $2}'` ; do kill -9 $i ; done
		done
	done
done


