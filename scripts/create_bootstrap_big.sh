#!/bin/bash

SCHED_HOME=/opt/hydra/Hydra
OUT=$SCHED_HOME/run/exercise_b_10/

export PATH=$PATH:$SCHED_HOME/src

extract_solution()
{
	file=$1

	enable=""
	while read line  ; do           
		if [ -z "$enable" ] ; then
			if [ "`echo -n $line`" == "decision variable:" ]; then
				enable="run"
			fi
		else
			echo $line
		fi
		
	done < $file 
}

for NTASKS in  `seq -w 50 50` ; do
	for ut in  `seq 75 75` ; do
		DIR=$OUT/$ut/$NTASKS/
		pushd $DIR 
		for file in `ls file????????.ga` ; do
			# only input solution
			cp ${file%.ga} $file.ga_input
			extract_solution $file >> $file.ga_input
			# only cut
			upper_cut=`head -n 3 ${file} | tail -n 1` 
			cp ${file%.ga} $file.ga_cut
			echo $upper_cut >> $file.ga_cut
			# both
			cp ${file%.ga} $file.ga_cut_input
			echo $upper_cut >> $file.ga_cut_input
			extract_solution $file >> $file.ga_cut_input
		done
		popd >& /dev/null
	done
done


