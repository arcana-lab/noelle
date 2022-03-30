#!/bin/bash

./scripts/printRunningTests.sh | sed 's/regression_[0-9]*\///g' | sort -u
