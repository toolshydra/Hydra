#!/bin/bash

SCHED_HOME=/opt/hydra/Hydra
SIM=$SCHED_HOME/src/hydra_gen
OUT=$SCHED_HOME/run/20160104_fixed/
MODELS=10000

LP=0.45

export PATH=$PATH:$SCHED_HOME/src


for t in lp ; do
	PARAM=""
	RANGES=$SCHED_HOME/models/simulation_${t}/modelsimperiodrange.txt
	for c in 1 ; do
		FREQS=$SCHED_HOME/models/simulation_${t}/modelsimfreqs-${c}cluster.txt
		SOLVERS_CONFIG=$SCHED_HOME/models/simulation_${t}/solvers_config.txt
#		for ut in  `seq 10 10 90` ; do
		for ut in  40 50 60 ; do
#			for NTASKS in  `seq -w 5 5 30` ; do
			for NTASKS in  20 25 30 ; do
				p=`echo "scale=8;4*($ut/100)" | bc`
				DIR=$OUT/$ut/$NTASKS/
				mkdir -p $DIR
				TEMPFS="$SIM --freq-file=$FREQS --range-file=$RANGES --processor-count=4 --u-total=$p --max-ui=$(echo "scale=4;($p / ($NTASKS * 0.6))" | bc) --task-count=$NTASKS --switch-latency=$LP $PARAM --solvers-file=$SOLVERS_CONFIG --model-count=$MODELS --output-location=$DIR"
				echo "$TEMPFS" > $DIR/result_${t}_${c}_${p}.txt
				$TEMPFS
			done
		done
	done
done


