#!/bin/bash

outputFile="$1" ;
echo "${@:2}" > $outputFile ;
