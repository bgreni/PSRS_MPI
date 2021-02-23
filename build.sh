cd src || exit 1
if [[ "$*" == *"clean"* ]]
then
  make clean
fi

source ~/.dotfiles/bash_functions

make singlethread
cpmpi 0 singlethread
MADE=$(make)
MADE_SUCC="$?"
if ! echo "$MADE" | grep -q "Nothing to be done";
then
  echo "$MADE"
  if [[ "$MADE_SUCC" == 0 ]] && [[ "$*" != *"nocopy"* ]]
  then
    cpmpi 0 main && \
    cpmpi 1 main && \
    cpmpi 2 main && \
    cpmpi 3 main
  fi
fi

ARR_SIZE=1000000
SEED=10
DO_VERIFY=1

if [[ "$*" == *"runremote"* ]] && [[ "$MADE_SUCC" == 0 ]]
then
  cd ..
  ./run.sh $ARR_SIZE $SEED $DO_VERIFY
fi

if [[ "$*" == *"runlocal"* ]] && [[ "$MADE_SUCC" == 0 ]]
then
  cd ..
  ./run.sh $ARR_SIZE $SEED $DO_VERIFY local
fi