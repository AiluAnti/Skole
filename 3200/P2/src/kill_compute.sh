#!/bin/bash

# Kills all of your processes on all compute nodes

for COMPUTE in $(rocks list host compute | cut -d: -f1 | tail -n +2)
do
    ssh -f $COMPUTE killall -u $(id -un)
done
