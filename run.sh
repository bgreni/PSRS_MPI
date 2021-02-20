if [[ "$3" == "local" ]]
then
  mpirun -np 8 src/main "$1" "$2"
else
  ssh -A ubuntu@10.2.9.148 "mpirun -f hosts -np 8  /home/ubuntu/main $1 $2"
fi