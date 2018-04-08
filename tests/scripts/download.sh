#!/bin/bash

for sw in "$@" ; do
  swName=`basename $sw` ;
  if ! test -d "$swName" ; then
    echo "Cloning $swName" ;
    git clone "$sw" ;
    continue ;
  fi

  echo "Updating $swName" ;
  pushd ./ ;
  cd "$swName" ;
  git pull ;
  popd ;
done
