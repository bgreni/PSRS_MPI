
#ARR_SIZE=1000000
#SEED=2
DO_VERIFY=0

truncate -s 0 outputhandling/results.txt

for ARR_SIZE in 2000000 3000000 4000000 5000000 32000000 #64000000 120000000
do
  for CORE_COUNT in 1 2 4 6 8
  do
    for SEED in 1 2 #3 4 5
    do
      if [[ $CORE_COUNT == 1 ]]
      then
        ssh -A ubuntu@10.2.9.148 "/home/ubuntu/singlethread '$ARR_SIZE' '$SEED'" >> outputhandling/results.txt
        echo "RUN $SEED OF CORE COUNT 1 OF ARR SIZE $ARR_SIZE HAS BEEN COMPLETED"
      else
        ssh -A ubuntu@10.2.9.148 "mpirun -f hosts -np $CORE_COUNT /home/ubuntu/main $ARR_SIZE $SEED $DO_VERIFY" >> outputhandling/results.txt
        printf "\n" >> outputhandling/results.txt
        echo "RUN $SEED OF CORE COUNT $CORE_COUNT OF ARR SIZE $ARR_SIZE HAS BEEN COMPLETED"
      fi
    done
  done
done
perl -pi -e 'chomp if eof' outputhandling/results.txt

./outputhandling/result_agg.py





#./run.sh $ARR_SIZE $SEED $DO_VERIFY >> outputhandling/results.txt
