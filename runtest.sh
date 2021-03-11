
set -e
DO_VERIFY=1

truncate -s 0 outputhandling/results.txt

for ARR_SIZE in 1000000 32000000 64000000 96000000 150000000
do
  for CORE_COUNT in 1 2 4 6 8
  do
    for SEED in 123 2234 3243 4234 52645
    do
      if [[ $CORE_COUNT == 1 ]]
      then
        ssh -A ubuntu@10.2.9.148 "/home/ubuntu/singlethread '$ARR_SIZE' '$SEED'" >> outputhandling/results.txt
        echo "RUN $SEED OF CORE COUNT 1 OF ARR SIZE $ARR_SIZE HAS BEEN COMPLETED"
      else
        HOSTFILE="ERROR"
        if [[ $CORE_COUNT == 8  ]]
        then
          HOSTFILE="hosts"
        elif [[ $CORE_COUNT == 6 ]]
        then
          HOSTFILE="hosts2"
        else
          HOSTFILE="hosts1"
        fi
        ssh -A ubuntu@10.2.9.148 "mpirun -f $HOSTFILE -np $CORE_COUNT /home/ubuntu/main $ARR_SIZE $SEED $DO_VERIFY" >> outputhandling/results.txt
        printf "\n" >> outputhandling/results.txt
        echo "RUN $SEED OF CORE COUNT $CORE_COUNT OF ARR SIZE $ARR_SIZE HAS BEEN COMPLETED"
      fi
    done
  done
done
perl -pi -e "chomp if eof" outputhandling/results.txt

./outputhandling/result_agg.py



