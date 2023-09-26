#!/bin/bash

stdbuf -oL mg5_aMC run.dat | tee run.log
