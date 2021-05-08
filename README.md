# PSRS MPI
An implementation of the PSRS distributed sorting algorithm using MPI for inter process communication

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

# Quick Explanation of Other Things
- The `outputhandling` folder contains the a couple python scripts I wrote to process the output data of the 
program, and to generate some nice graphs and tables, as well as the actual image files that it generated. No need to
  look through it unless you wanna see some hacked together garbage.
   
- All the scripts, the `build.sh` and `run.sh` scripts are the only ones you need to build and run the program, but the 
`runtestlocal.sh` script can also be used if you wish to inspect the parameters I used for the test suite. Yay for bash!
