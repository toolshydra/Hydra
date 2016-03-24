#!/bin/bash

SCHED_HOME=/opt/hydra/Hydra
SIM_EXACT=$SCHED_HOME/src/solver_mgap_rm_exact
SIM_TIGHT=$SCHED_HOME/src/solver_mgap_edf_tight
OUT=$SCHED_HOME/run/20151208_fixed/exac

LP=0.45

export PATH=$PATH:$SCHED_HOME/src


init=$(date)
for t in lp ; do
	PARAM=""
	RANGES=$SCHED_HOME/models/simulation_${t}/modelsimperiodrange.txt
	for c in 1 ; do
		FREQS=$SCHED_HOME/models/simulation_${t}/modelsimfreqs-${c}cluster.txt
		SOLVERS_CONFIG=$SCHED_HOME/models/simulation_${t}/solvers_config.txt
		for NTASKS in  `seq -w 5 5 30` ; do
			for ut in  `seq 10 10 90` ; do
				DIR=$OUT/$ut/$NTASKS/
				pushd $DIR
				for file in `ls file???????? | tail -100` ; do
					TEMPFS="$SIM_EXACT -tm $file"
					echo "$DIR:$TEMPFS ($init $(date))"
					$TEMPFS > $file.exact
					#TEMPFS="$SIM_TIGHT -tm $file"
					#echo "$DIR:$TEMPFS ($init $(date))"
					#$TEMPFS > $file.tight
				done
				popd
			done
		done
	done
done


