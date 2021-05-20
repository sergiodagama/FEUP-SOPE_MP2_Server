#!/usr/bin/env bash

###25abr corrections!

echo ":::::::: SOPE 2020/2021 MT2 ::::::::"
echo ":::::::: VALIDATION SCRIPT ::::::::"

main() {
	initVars

	case "$1" in
		clean) clean; exit;;
		1) echo ":::: Test case $1 - Server outlives client" ;
			clean;
			run_server -t 20 -l 10 /tmp/fifo_"${USER}";
			sleep 1 ;
			run_client -t 10 /tmp/fifo_"${USER}";;
		2) echo ":::: Test case $1 - Server dies first";
			clean;
			run_server -t 5 -l 10 /tmp/fifo_"${USER}";
			sleep 1 ;
			run_client -t 10 /tmp/fifo_"${USER}";;
		3) echo ":::: Test case $1 - Server starts late";
			clean;
			run_client -t 10 /tmp/fifo_"${USER}";
			sleep 1 ;
			run_server -t 20 -l 10 /tmp/fifo_"${USER}";;
		*)  echo "Usage: $0 <test_no> || $0 clean" ;
			exit 1 ;;
	esac

	postProcessing
	countTotals
	verifyTotals
	getSequences
	verifySequences
}

initVars() {
srv_pid=-1
srv_args=""
cli_pid=-1
srv_args=""
}

clean() {
  echo "Cleaning logs"
  rm -rf /tmp/fifo_"${USER}"
  rm -f server_log.txt client_log.txt
  rm -f client_stderr.txt server_stderr.txt
}

run_server() { ./s "$@" > server_log.txt 2> server_stderr.txt & srv_pid=$!; srv_args="./s $@"; }
run_client() { ./c "$@" > client_log.txt 2> client_stderr.txt & cli_pid=$!; cli_args="./c $@"; }

postProcessing() {
sleep 1
echo "Running ${cli_args}  ${srv_args} ..."

wait $cli_pid
echo "Client PID: $cli_pid, exit status: $?, cmd: ${cli_args}"

wait $srv_pid
echo "Server PID: $srv_pid, exit status: $?, cmd: ${srv_args}"

rm -rf /tmp/fifo_"${USER}"

echo "Assessing..."
sleep 2
}

countTotals() {
#Count client
total_IWANT=$(grep -o -i IWANT client_log.txt | wc -l)
total_GOTRS=$(grep -o -i GOTRS client_log.txt | wc -l)
total_CLOSD=$(grep -o -i CLOSD client_log.txt | wc -l)
total_GAVUP=$(grep -o -i GAVUP client_log.txt | wc -l)

#Count server
total_RECVD=$(grep -o -i RECVD server_log.txt | wc -l)
total_TSKEX=$(grep -o -i TSKEX server_log.txt | wc -l)
total_TSKDN=$(grep -o -i TSKDN server_log.txt | wc -l)
total_2LATE=$(grep -o -i 2LATE server_log.txt | wc -l)
total_FAILD=$(grep -o -i FAILD server_log.txt | wc -l)
}

#Consistency client
# total_IWANT = total_GOTRS + total_CLOSD + total_GAVUP

#Consistency server
# total_RECVD = total_TSKDN + total_2LATE + total_FAILD
###25abr incorrecto: # total_RECVD = total_TSKEX + total_2LATE + total_FAILD
###25abr incorrecto: # total_TSKEX = total_TSKDN

#Consistency client-server
# total_IWANT = total_RECVD
# total_GOTRS = total_TSKDN
# total_CLOSD = total_2LATE
# total_GAVUP =?= total_FAILD

getSequences() {
#Right and unique sequence
RES_CLIENT=$(grep 'GOTRS' client_log.txt | tr -d [:blank:] | awk -F\; '{print $6}' | sort -n)
RES_EXEC=$(grep 'TSKEX' server_log.txt | tr -d [:blank:] | awk -F\; '{print $6}' | sort -n)
RES_DONE=$(grep 'TSKDN' server_log.txt | tr -d [:blank:] | awk -F\; '{print $6}' | sort -n)
RES_DONE_FAILD=$(grep 'TSKDN\|FAILD' server_log.txt | tr -d [:blank:] | awk -F\; '{print $6}' | sort -n)

ID_CLIENT=$(grep 'IWANT' client_log.txt | tr -d [:blank:] | awk -F\; '{print $2}' | sort -n)
ID_SERVER=$(grep 'RECVD' server_log.txt | tr -d [:blank:] | awk -F\; '{print $2}' | sort -n)
ID_EXEC=$(grep 'TSKEX' server_log.txt | tr -d [:blank:] | awk -F\; '{print $2}' | sort -n)
ID_DONE=$(grep 'TSKDN' server_log.txt | tr -d [:blank:] | awk -F\; '{print $2}' | sort -n)
ID_EXEC_2LATE=$(grep 'TSKEX\|2LATE' server_log.txt | tr -d [:blank:] | awk -F\; '{print $2}' | sort -n)
ID_DONE_FAILD=$(grep 'TSKDN\|FAILD' server_log.txt | tr -d [:blank:] | awk -F\; '{print $2}' | sort -n)
}

 verifyTotals() {
  echo "TOTALS CHECK"
  echo "Client IWANT: $total_IWANT, Client GOTRS: $total_GOTRS, Client CLOSD: $total_CLOSD, Client GAVUP: $total_GAVUP"
  if [ "$total_IWANT" -eq $(($total_GOTRS + $total_CLOSD + $total_GAVUP)) ]; then echo "ALL OK"; else echo "ERROR (IWANT-GOTRS-CLOSD-GAVUP)"; fi

  echo "Server RECVD: $total_RECVD, Server TSKEX: $total_TSKEX, Server TSKDN: $total_TSKDN, Server 2LATE: $total_2LATE, Server FAILD: $total_FAILD"
  if [ "$total_RECVD" -eq $(($total_TSKDN + $total_2LATE + $total_FAILD)) ]; then echo "ALL OK"; else echo "ERROR (RECVD-TSKDN-2LATE-FAILD)"; fi	###25abr
###25abr  if [ "$total_TSKEX" -eq "$total_TSKDN" ]; then echo "ALL OK"; else echo "ERROR (TSKEX-TSKDN)"; fi

  if [ "$total_IWANT" -eq "$total_RECVD" ]; then echo "ALL OK"; else echo "ERROR (IWANT-RECVD)"; fi
  if [ "$total_GOTRS" -eq "$total_TSKDN" ]; then echo "ALL OK"; else echo "ERROR (GOTRS-TSKDN)"; fi
  if [ "$total_CLOSD" -eq "$total_2LATE" ]; then echo "ALL OK"; else echo "ERROR (CLOSD-2LATE)"; fi
  if [ "$total_GAVUP" -eq "$total_FAILD" ]; then echo "ALL OK"; else echo "ERROR (GAVUP-FAILD)"; fi
 }

verifySequences() {
 echo "TASKS CHECK"
  UNIQUE_IDS_CLIENT=$(echo "$ID_CLIENT" | uniq -d)
  if [ ! -z "$UNIQUE_IDS_CLIENT" ]; then echo "ERROR: REPEATED IDS ON CLIENT"; else echo "ALL OK (CLIENT-IDS)"; fi
  UNIQUE_RES_CLIENT=$(echo "$RES_CLIENT" | uniq -d)
  if [ ! -z "$UNIQUE_RES_CLIENT" ]; then echo "ERROR: REPEATED RESULTS ON CLIENT"; else echo "ALL OK (CLIENT-RES)"; fi

  UNIQUE_IDS_SERVER=$(echo "$ID_SERVER" | uniq -d)
  if [ ! -z "$UNIQUE_IDS_SERVER" ]; then echo "ERROR: REPEATED IDS ON SERVER"; else echo "ALL OK (SERVER-IDS)"; fi
  UNIQUE_RES_SERVER=$(echo "$RES_DONE" | uniq -d)
  if [ ! -z "$UNIQUE_RES_SERVER" ]; then echo "ERROR: REPEATED RESULTS ON SERVER"; else echo "ALL OK (SERVER-RES)"; fi

  if [ "$ID_CLIENT" = "$ID_EXEC_2LATE" ]; then echo "ALL OK"; else echo "ERROR: OPERATIONS EXECUTED DIFFER ON CLIENT vs SERVER"; fi
  if [ "$RES_CLIENT" = "$RES_DONE" ]; then echo "ALL OK"; else echo "ERROR: RESULTS RETURNED DIFFER ON CLIENT vs SERVER"; fi
###25abr  if [ "$RES_CLIENT" = "$RES_EXEC" ]; then echo "ALL OK"; else echo "ERROR: RESULTS RETURNED DIFFER ON CLIENT vs SERVER"; fi
###25abr  if [ "$RES_EXEC" = "$RES_DONE" ]; then echo "ALL OK"; else echo "ERROR: RESULTS RECEIVED DIFFER FROM SENT"; fi
}

main "$@"
