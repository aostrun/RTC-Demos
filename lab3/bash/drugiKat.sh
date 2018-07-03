#!/bin/bash

if [ "$1" = "gore" ]; then
  echo "17" >> ../pins.txt
fi

if [ "$1" = "dole" ]; then
  echo "+17" >> ../pins.txt
fi
