#!/bin/bash

if [ "$1" = "gore" ]; then
  echo "22" >> ../pins.txt
fi

if [ "$1" = "dole" ]; then
  echo "+22" >> ../pins.txt
fi
