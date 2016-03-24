#!/bin/bash

SCHED_HOME=/opt/hydra/Hydra
OUT=$SCHED_HOME/run/exercise_b_10/

LP=0.45

export PATH=$PATH:$SCHED_HOME/src

echo "#rm_resp,rm_util,edf_util,rlx_rm_resp,rlx_rm_util,rlx_edf_util,rmresp_ga,rmutil_ga,edfutil_ga,ga_rm_resp,ga_rm_util,ga_edf_util"
echo "filename,modtime,u_total,tasks,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap,ok,time,energy,u_total_final,gap"

init=$(date)
for t in lp ; do
	PARAM=""
	RANGES=$SCHED_HOME/models/simulation_${t}/modelsimperiodrange.txt
	for c in 1 ; do
		FREQS=$SCHED_HOME/models/simulation_${t}/modelsimfreqs-${c}cluster.txt
		SOLVERS_CONFIG=$SCHED_HOME/models/simulation_${t}/solvers_config.txt
		for NTASKS in  `seq -w 50 50` ; do
			for ut in  `seq 60 60` ; do
				DIR=$OUT/$ut/$NTASKS/
				pushd $DIR 
				for file in file5HPzJAru fileasbke0DO fileqNnTqt2P ; do
				for i in `seq 4` ; do
					exact_ok=`head -1 $file.rm_resp.$i`
					exact_time=`head -2 $file.rm_resp.$i | tail -1`
					exact_energy=`head -3 $file.rm_resp.$i | tail -1`
					exact_gap=`head -4 $file.rm_resp.$i | tail -1`
					exact_utot=`grep "Total System Utilization" $file.rm_resp.$i | awk '{ print $4 }'`


					util_ok=`head -1 $file.rm_util.$i`
					util_time=`head -2 $file.rm_util.$i | tail -1`
					util_energy=`head -3 $file.rm_util.$i | tail -1`
					util_gap=`head -4 $file.rm_util.$i | tail -1`
					util_utot=`grep "Total System Utilization" $file.rm_util.$i | awk '{ print $4 }'`

					edf_ok=`head -1 $file.edf_util.$i`
					edf_time=`head -2 $file.edf_util.$i | tail -1`
					edf_energy=`head -3 $file.edf_util.$i | tail -1`
					edf_gap=`head -4 $file.edf_util.$i | tail -1`
					edf_utot=`grep "Total System Utilization" $file.edf_util.$i | awk '{ print $4 }'`

					garmresp_ok=`head -1 $file.ga_rmresp`
					garmresp_time=`head -2 $file.ga_rmresp | tail -1`
					garmresp_energy=`head -3 $file.ga_rmresp | tail -1`
					garmresp_gap=`head -4 $file.ga_rmresp | tail -1`
					garmresp_utot=`grep "Total System Utilization" $file.ga_rmresp | awk '{ print $4 }'`

					garmutil_ok=`head -1 $file.ga_rmutil`
					garmutil_time=`head -2 $file.ga_rmutil | tail -1`
					garmutil_energy=`head -3 $file.ga_rmutil | tail -1`
					garmutil_gap=`head -4 $file.ga_rmutil | tail -1`
					garmutil_utot=`grep "Total System Utilization" $file.ga_rmutil | awk '{ print $4 }'`

					ga_ok=`head -1 $file.ga_edfutil`
					ga_time=`head -2 $file.ga_edfutil | tail -1`
					ga_energy=`head -3 $file.ga_edfutil | tail -1`
					ga_gap=`head -4 $file.ga_edfutil | tail -1`
					ga_utot=`grep "Total System Utilization" $file.ga_edfutil | awk '{ print $4 }'`


					ega_ok=`head -1 $file.ga_edfutil.ga_cut_input.edf.$i`
					ega_time=`head -2 $file.ga_edfutil.ga_cut_input.edf.$i | tail -1`
					ega_energy=`head -3 $file.ga_edfutil.ga_cut_input.edf.$i | tail -1`
					ega_gap=`head -4 $file.ga_edfutil.ga_cut_input.edf.$i | tail -1`
					ega_utot=`grep "Total System Utilization" $file.ga_edfutil.ga_cut_input.edf.$i | awk '{ print $4 }'`
					

					ruga_ok=`head -1 $file.ga_rmutil.ga_rmutil_cut_input.rm_util.$i`
					ruga_time=`head -2 $file.ga_rmutil.ga_rmutil_cut_input.rm_util.$i | tail -1`
					ruga_energy=`head -3 $file.ga_rmutil.ga_rmutil_cut_input.rm_util.$i | tail -1`
					ruga_gap=`head -4 $file.ga_rmutil.ga_rmutil_cut_input.rm_util.$i | tail -1`
					ruga_utot=`grep "Total System Utilization" $file.ga_rmutil.ga_rmutil_cut_input.rm_util.$i | awk '{ print $4 }'`


					rrga_ok=`head -1 $file.ga_rmresp.ga_rmresp_cut_input.rm_resp.$i`
					rrga_time=`head -2 $file.ga_rmresp.ga_rmresp_cut_input.rm_resp.$i | tail -1`
					rrga_energy=`head -3 $file.ga_rmresp.ga_rmresp_cut_input.rm_resp.$i | tail -1`
					rrga_gap=`head -4 $file.ga_rmresp.ga_rmresp_cut_input.rm_resp.$i | tail -1`
					rrga_utot=`grep "Total System Utilization" $file.ga_rmresp.ga_rmresp_cut_input.rm_resp.$i | awk '{ print $4 }'`
					modfile=`stat -c %y "$file"`

					echo "$file,$modfile,$ut,$NTASKS,$exact_ok,$exact_time,$exact_energy,$exact_utot,$exact_gap,$util_ok,$util_time,$util_energy,$util_utot,$util_gap,$edf_ok,$edf_time,$edf_energy,$edf_utot,$edf_gap,$rexact_ok,$rexact_time,$rexact_energy,$rexact_utot,$rexact_gap,$rutil_ok,$rutil_time,$rutil_energy,$rutil_utot,$rutil_gap,$redf_ok,$redf_time,$redf_energy,$redf_utot,$redf_gap,$garmresp_ok,$garmresp_time,$garmresp_energy,$garmresp_utot,inf,$garmutil_ok,$garmutil_time,$garmutil_energy,$garmutil_utot,inf,$ga_ok,$ga_time,$ga_energy,$ga_utot,inf,$rrga_ok,$rrga_time,$rrga_energy,$rrga_utot,$rrga_gap,$ruga_ok,$ruga_time,$ruga_energy,$ruga_utot,$ruga_gap,$ega_ok,$ega_time,$ega_energy,$ega_utot,$ega_gap"


				done
				done
				popd >& /dev/null
			done
		done
	done
done


