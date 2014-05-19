#!/bin/bash
AK_HOME=$HOME/local/akaroa
AKRUN=$AK_HOME/bin/akrun
AK_INSTANCES=16
AK_PARAMS=models/ak_params.txt

SCHED_HOME=/Users/evalentin/Documents/doutorado/algorithms/schedulability
SIM=$SCHED_HOME/src/pseudosim
LOG=$SCHED_HOME/results/time

LP=0.45

for t in lp no_lp ; do
#for t in lp; do
	if [ "$t" == "lp" ] ; then
		PARAM=""
		NTASKS=30
	else
		PARAM="--compare-no-lp"
		NTASKS=30
	fi
	RANGES=$SCHED_HOME/models/simulation_$t/modelsimranges.txt
	for c in 1 2 ; do
		FREQS=$SCHED_HOME/models/simulation_$t/modelsimfreqs-${c}cluster.txt
		for p in 16 ; do
			TMPFS=$(mktemp -t sim)
			echo "#!/bin/bash" > $TMPFS
			echo "$SIM --freq-file=$FREQS --range-file=$RANGES --processor-count=$p --task-count=$NTASKS --switch-latency=$LP $PARAM" >> $TMPFS
			chmod +x $TMPFS
			echo "$SIM --freq-file=$FREQS --range-file=$RANGES --processor-count=$p --task-count=$NTASKS --switch-latency=$LP $PARAM" > $LOG/result_${t}_${c}_${p}.txt
			$AKRUN -n $AK_INSTANCES -s -f $AK_PARAMS $TMPFS >> $LOG/result_${t}_${c}_${p}.txt
			rm $TMPFS
			#somehow we are not able to clean this properly
			for i in  `ps aux | grep pseudosim | awk '{print $2}'` ; do kill -9 $i ; done
		done
	done
done


