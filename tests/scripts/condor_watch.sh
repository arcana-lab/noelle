#!/bin/bash

currentDir="\"`pwd`\"" ;
constraint="Iwd == ${currentDir}";

watch condor_q `whoami` -constraint \'$constraint\' ;
