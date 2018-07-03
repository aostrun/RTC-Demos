#!/bin/bash

if [ "$1" = "gore" ]; then
  echo "27" >> ../pins.txt
fi

if [ "$1" = "dole" ]; then
  echo "+27" >> ../pins.txt
fi
