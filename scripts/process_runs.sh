#!/bin/bash

SCHED_HOME=/opt/hydra/Hydra
OUT=/tmp/exac

LP=0.45

export PATH=$PATH:$SCHED_HOME/src

echo "filename,modtime,u_total,tasks,algo,ok,time,energy,u_total_final"

init=$(date)
for t in lp ; do
	PARAM=""
	RANGES=$SCHED_HOME/models/simulation_${t}/modelsimperiodrange.txt
	for c in 1 ; do
		FREQS=$SCHED_HOME/models/simulation_${t}/modelsimfreqs-${c}cluster.txt
		SOLVERS_CONFIG=$SCHED_HOME/models/simulation_${t}/solvers_config.txt
		for NTASKS in  `seq -w 5 5 30` ; do
			for ut in  `seq 10 10 60` ; do
				DIR=$OUT/$ut/$NTASKS/
				pushd $DIR >& /dev/null
				for file in `ls file????????` ; do
					if [ -e "$file.exact" ] ; then
					exact_ok=`head -1 $file.exact`
					exact_time=`head -2 $file.exact | tail -1`
					exact_energy=`head -3 $file.exact | tail -1`
					exact_utot=`tail -1 $file.exact | awk '{ print $4 }'`
					fi


					if [ -e "$file.tight" ] ; then
					util_ok=`head -1 $file.tight`
					util_time=`head -2 $file.tight | tail -1`
					util_energy=`head -3 $file.tight | tail -1`
					util_utot=`tail -1 $file.tight | awk '{ print $4 }'`
					fi

					if [ -e "$file.edf" ] ; then
					edf_ok=`head -1 $file.edf`
					edf_time=`head -2 $file.edf | tail -1`
					edf_energy=`head -3 $file.edf | tail -1`
					edf_utot=`tail -1 $file.edf | awk '{ print $4 }'`
					fi



					modfile=`stat -c %y "$file"`

					echo "$file,$modfile,$ut,$NTASKS,exact,$exact_ok,$exact_time,$exact_energy,$exact_utot"
					echo "$file,$modfile,$ut,$NTASKS,util,$util_ok,$util_time,$util_energy,$util_utot"
					echo "$file,$modfile,$ut,$NTASKS,edf,$edf_ok,$edf_time,$edf_energy,$edf_utot"

				done
				popd >& /dev/null
			done
		done
	done
done


