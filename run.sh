if [[ "$4" == "local" ]]
then
  mpirun -np 8 src/main "$1" "$2 $3"
else
  ssh -A ubuntu@10.2.9.148 "mpirun -f hosts -np 6  /home/ubuntu/main $1 $2 $3"
fi