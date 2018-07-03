#!/bin/bash

if [ $1 = 0 ]; then

  for (( i = 1; i <= $2; i++ )) ; do
      echo "31" >> ../pins.txt
      sleep .8
  done

fi

if [ $1 = 1 ]; then

  for (( i = 1; i <= $2; i++ )) ; do
      echo "7" >> ../pins.txt
      sleep .8
  done

fi
