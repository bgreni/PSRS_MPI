# Running the program
on the off chance you want to actually run the project, follow these steps from the 
root folder of the project

1. Build from source (using nocopy flag so it won't try and copy it to the remote
   machines)
```commandline
./build.sh nocopy
```
2. Run the program, replacing the first 3 args with the desired parameters
```commandline
./run.sh $ARR_SIZE $SEED $DO_VERIFY local
```