#!/bin/bash

SCHED_HOME=/opt/hydra/Hydra
OUT=/tmp/exac

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
		for NTASKS in  `seq -w 5 5 30` ; do
			for ut in  `seq 10 10 60` ; do
				DIR=$OUT/$ut/$NTASKS/
				pushd $DIR >& /dev/null
				for file in `ls file????????` ; do
					if [ -e "$file.exact" ] ; then
					exact_ok=`head -1 $file.exact`
					exact_time=`head -2 $file.exact | tail -1`
					exact_energy=`head -3 $file.exact | tail -1`
					exact_gap=`head -4 $file.exact | tail -1`
					exact_utot=`grep "Total System Utilization" $file.exact | awk '{ print $4 }'`
					fi


					if [ -e "$file.tight" ] ; then
					util_ok=`head -1 $file.tight`
					util_time=`head -2 $file.tight | tail -1`
					util_energy=`head -3 $file.tight | tail -1`
					util_gap=`head -4 $file.tight | tail -1`
					util_utot=`grep "Total System Utilization" $file.tight | awk '{ print $4 }'`
					fi

					if [ -e "$file.edf" ] ; then
					edf_ok=`head -1 $file.edf`
					edf_time=`head -2 $file.edf | tail -1`
					edf_energy=`head -3 $file.edf | tail -1`
					edf_gap=`head -4 $file.edf | tail -1`
					edf_utot=`grep "Total System Utilization" $file.edf | awk '{ print $4 }'`
					fi

					if [ -e "$file.r.exact" ] ; then
					rexact_ok=`head -1 $file.r.exact`
					rexact_time=`head -2 $file.r.exact | tail -1`
					rexact_energy=`head -3 $file.r.exact | tail -1`
					rexact_gap=`head -4 $file.r.edf | tail -1`
					rexact_utot=`grep "Total System Utilization" $file.r.exact | awk '{ print $4 }'`
					fi


					if [ -e "$file.r.tight" ] ; then
					rutil_ok=`head -1 $file.r.tight`
					rutil_time=`head -2 $file.r.tight | tail -1`
					rutil_energy=`head -3 $file.r.tight | tail -1`
					rutil_gap=`head -4 $file.r.tight | tail -1`
					rutil_utot=`grep "Total System Utilization" $file.r.tight | awk '{ print $4 }'`
					fi

					if [ -e "$file.r.edf" ] ; then
					redf_ok=`head -1 $file.r.edf`
					redf_time=`head -2 $file.r.edf | tail -1`
					redf_energy=`head -3 $file.r.edf | tail -1`
					redf_gap=`head -4 $file.r.edf | tail -1`
					redf_utot=`grep "Total System Utilization" $file.r.edf | awk '{ print $4 }'`
					fi

					if [ -e "$file.ga_rmresp" ] ; then
					garmresp_ok=`head -1 $file.ga_rmresp`
					garmresp_time=`head -2 $file.ga_rmresp | tail -1`
					garmresp_energy=`head -3 $file.ga_rmresp | tail -1`
					garmresp_gap=`head -4 $file.ga_rmresp | tail -1`
					garmresp_utot=`grep "Total System Utilization" $file.ga_rmresp | awk '{ print $4 }'`
					fi

					if [ -e "$file.ga_rmutil" ] ; then
					garmutil_ok=`head -1 $file.ga_rmutil`
					garmutil_time=`head -2 $file.ga_rmutil | tail -1`
					garmutil_energy=`head -3 $file.ga_rmutil | tail -1`
					garmutil_gap=`head -4 $file.ga_rmutil | tail -1`
					garmutil_utot=`grep "Total System Utilization" $file.ga_rmutil | awk '{ print $4 }'`
					fi

					if [ -e "$file.ga" ] ; then
					ga_ok=`head -1 $file.ga`
					ga_time=`head -2 $file.ga | tail -1`
					ga_energy=`head -3 $file.ga | tail -1`
					ga_gap=`head -4 $file.ga | tail -1`
					ga_utot=`grep "Total System Utilization" $file.ga | awk '{ print $4 }'`
					fi

					if [ -e "$file.ga.ga_cut_input.edf" ] ; then
					ega_ok=`head -1 $file.ga.ga_cut_input.edf`
					ega_time=`head -2 $file.ga.ga_cut_input.edf | tail -1`
					ega_energy=`head -3 $file.ga.ga_cut_input.edf | tail -1`
					ega_gap=`head -4 $file.ga.ga_cut_input.edf | tail -1`
					ega_utot=`grep "Total System Utilization" $file.ga.ga_cut_input.edf | awk '{ print $4 }'`
					fi
					

					if [ -e "$file.ga_rmutil.ga_rmutil_cut_input.tight" ] ; then
					ruga_ok=`head -1 $file.ga_rmutil.ga_rmutil_cut_input.tight`
					ruga_time=`head -2 $file.ga_rmutil.ga_rmutil_cut_input.tight | tail -1`
					ruga_energy=`head -3 $file.ga_rmutil.ga_rmutil_cut_input.tight | tail -1`
					ruga_gap=`head -4 $file.ga_rmutil.ga_rmutil_cut_input.tight | tail -1`
					ruga_utot=`grep "Total System Utilization" $file.ga_rmutil.ga_rmutil_cut_input.tight | awk '{ print $4 }'`
					fi


					if [ -e "$file.ga_rmresp.ga_rmresp_cut_input.exact" ] ; then
					rrga_ok=`head -1 $file.ga_rmresp.ga_rmresp_cut_input.exact`
					rrga_time=`head -2 $file.ga_rmresp.ga_rmresp_cut_input.exact | tail -1`
					rrga_energy=`head -3 $file.ga_rmresp.ga_rmresp_cut_input.exact | tail -1`
					rrga_gap=`head -4 $file.ga_rmresp.ga_rmresp_cut_input.exact | tail -1`
					rrga_utot=`grep "Total System Utilization" $file.ga_rmresp.ga_rmresp_cut_input.exact | awk '{ print $4 }'`
					fi
					modfile=`stat -c %y "$file"`

					echo "$file,$modfile,$ut,$NTASKS,$exact_ok,$exact_time,$exact_energy,$exact_utot,$exact_gap,$util_ok,$util_time,$util_energy,$util_utot,$util_gap,$edf_ok,$edf_time,$edf_energy,$edf_utot,$edf_gap,$rexact_ok,$rexact_time,$rexact_energy,$rexact_utot,$rexact_gap,$rutil_ok,$rutil_time,$rutil_energy,$rutil_utot,$rutil_gap,$redf_ok,$redf_time,$redf_energy,$redf_utot,$redf_gap,$garmresp_ok,$garmresp_time,$garmresp_energy,$garmresp_utot,inf,$garmutil_ok,$garmutil_time,$garmutil_energy,$garmutil_utot,inf,$ga_ok,$ga_time,$ga_energy,$ga_utot,inf,$rrga_ok,$rrga_time,$rrga_energy,$rrga_utot,$rrga_gap,$ruga_ok,$ruga_time,$ruga_energy,$ruga_utot,$ruga_gap,$ega_ok,$ega_time,$ega_energy,$ega_utot,$ega_gap"


				done
				popd >& /dev/null
			done
		done
	done
done


