#! /bin/bash

FREQ_FILE=freqs.txt
RANGE_FILE=range.txt
NFREQ=5
NRES=5
SIMU=$(pwd)/../src/pseudosim
ARGS="-m $NFREQ -p $NRES -f $FREQ_FILE -r $RANGE_FILE"
AKAROA_HOME=/usr/local/akaroa/bin
AKRUN=$AKAROA_HOME/akrun
AKMASTER=$AKAROA_HOME/akmaster
AKSLAVE=$AKAROA_HOME/akslave
NSIMU=4

# A  -i  --best-initial-limits         Removes combinations by limiting too low frequencies.
# B  -k  --start-drop                  Drop initial useless samples.
# C  -j  --jump-useless                Jump useless detected samples.

function exec_simul {
	NTASK=$1

	echo "Generating simulation batch for $NTASK tasks"
	echo "Executing type -"
	echo "$AKRUN -n $NSIMU -- $SIMU $ARGS -n $NTASK >>& $NTASK.-.log"
	date > $NTASK.-.log
	$AKRUN -n $NSIMU -- $SIMU $ARGS -n $NTASK >>& $NTASK.-.log
	date >> $NTASK.-.log
	echo "Executing type A"
	echo "$AKRUN -n $NSIMU -- $SIMU $ARGS -i -n $NTASK  >>& $NTASK.A.log"
	date > $NTASK.A.log
	$AKRUN -n $NSIMU -- $SIMU $ARGS -i -n $NTASK  >>& $NTASK.A.log
	date >> $NTASK.A.log
	echo "Executing type B"
	echo "$AKRUN -n $NSIMU -- $SIMU $ARGS -k -n $NTASK  >>& $NTASK.B.log"
	date > $NTASK.B.log
	$AKRUN -n $NSIMU -- $SIMU $ARGS -k -n $NTASK >>& $NTASK.B.log
	date >> $NTASK.B.log
	echo "Executing type C"
	echo "$AKRUN -n $NSIMU -- $SIMU $ARGS -j -n $NTASK >>& $NTASK.C.log"
	date > $NTASK.C.log
	$AKRUN -n $NSIMU -- $SIMU $ARGS -j -n $NTASK  >>& $NTASK.C.log
	date >> $NTASK.C.log
	echo "Executing type A+B"
	echo "$AKRUN -n $NSIMU -- $SIMU $ARGS -i -k -n $NTASK  >>& $NTASK.A+B.log"
	date > $NTASK.A+B.log
	$AKRUN -n $NSIMU -- $SIMU $ARGS -i -k -n $NTASK  >>& $NTASK.A+B.log
	date >> $NTASK.A+B.log
	echo "Executing type A+C"
	echo "$AKRUN -n $NSIMU -- $SIMU $ARGS -i -j -n $NTASK  >>& $NTASK.A+C.log"
	date > $NTASK.A+C.log
	$AKRUN -n $NSIMU -- $SIMU $ARGS -i -j -n $NTASK  >>& $NTASK.A+C.log
	date >> $NTASK.A+C.log
	echo "Executing type B+C"
	echo "$AKRUN -n $NSIMU -- $SIMU $ARGS -k -j -n $NTASK  >>& $NTASK.B+C.log"
	date > $NTASK.B+C.log
	$AKRUN -n $NSIMU -- $SIMU $ARGS -k -j -n $NTASK  >>& $NTASK.B+C.log
	date >> $NTASK.B+C.log
	echo "Executing type A+B+C"
	echo "$AKRUN -n $NSIMU -- $SIMU $ARGS -i -k -j -n $NTASK  >>& $NTASK.A+B+C.log"
	date > $NTASK.A+B+C.log
	$AKRUN -n $NSIMU -- $SIMU $ARGS -i -k -j -n $NTASK  >>& $NTASK.A+B+C.log
	date >> $NTASK.A+B+C.log
}

for n in `seq 5 $1` ; do
	exec_simul $n
done

