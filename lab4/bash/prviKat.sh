#!/bin/bash
echo $1
if [ "$1" = "gore" ]; then
  echo "18" >> ../pins.txt
fi
