
set -e

truncate -s 0 outputhandling/localresults.txt
DO_VERIFY=1

for ARR_SIZE in 1000000 32000000 64000000 96000000
do
  for CORE_COUNT in 1 2 4 6 8
  do
    for SEED in 123 2234 3243 4234 52645
    do
      if [[ $CORE_COUNT == 1 ]]
      then
        ./src/singlethread $ARR_SIZE $SEED >> outputhandling/localresults.txt
        echo "LOCAL RUN $SEED OF CORE COUNT 1 OF ARR SIZE $ARR_SIZE HAS BEEN COMPLETED"
      else
        mpirun -np $CORE_COUNT src/main $ARR_SIZE $SEED $DO_VERIFY >> outputhandling/localresults.txt
        printf "\n" >> outputhandling/localresults.txt
        echo "LOCAL RUN $SEED OF CORE COUNT $CORE_COUNT OF ARR SIZE $ARR_SIZE HAS BEEN COMPLETED"
      fi
    done
  done
done

perl -pi -e "chomp if eof" outputhandling/localresults.txt

./outputhandling/result_agg.py --local