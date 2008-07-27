#!/bin/sh
# Runs gesd

export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH
exec ./gesd --wii --config ../config/amarok.ges
