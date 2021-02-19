if [[ "$2" == "local" ]]
then
  mpirun -np 3 src/main "$1"
else
  ssh -A ubuntu@10.2.9.148 "mpirun -np 4 -f hosts /home/ubuntu/main $1"
fi