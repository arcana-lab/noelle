#!/bin/bash

cat regression*/errors.txt | awk '{print $1}' | sed 's/.*regression_[0-9]*\///' | sort -u
