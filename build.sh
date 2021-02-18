cd src || exit 1
if [[ "$*" == "clean" ]]
then
  make clean
fi

source ~/.dotfiles/bash_functions

MADE=$(make)
MADE_SUCC="$?"
if ! echo "$MADE" | grep -q "Nothing to be done";
then
  echo "$MADE"
  if [[ "$MADE_SUCC" == 0 ]]
  then
    cpmpi 0 main && \
    cpmpi 1 main && \
    cpmpi 2 main && \
    cpmpi 3 main
  fi
fi


ARR_SIZE=1000


if [[ "$*" == "run" ]] && [[ "$MADE_SUCC" == 0 ]]
then
  cd ..
  ./run.sh 1000
fi