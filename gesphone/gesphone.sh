#!/bin/sh
# Runs gesphone

export LD_LIBRARY_PATH=../ges/Debug:$LD_LIBRARY_PATH
exec ./Debug/gesphone $*