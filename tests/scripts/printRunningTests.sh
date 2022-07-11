#!/bin/bash

condor_q `whoami` -l | grep "`pwd`" | grep regression_ | grep Argum | awk '{print $4}' | sed 's/.*tests\/regression/regression/g' ;
