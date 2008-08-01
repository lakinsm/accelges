#!/bin/sh

export SERVER_NAME=google.ro
export ACCELS_DIR=~/accels-$USERNAME-$RANDOM

echo "(C) 2008 Openmoko Inc. Paul-Valentin Borza <paul@borza.ro>"
echo
echo "Call for Help: Training of Gestures"
echo "  You'll have to make several gestures, and your recorded"
echo "  accelerations will be uploaded to '$SERVER_NAME' automatically."
echo "  Thank you for contributing!"
echo
echo "Checking connectivity to '$SERVER_NAME':"
if ping $SERVER_NAME -c 4 1> /dev/null 2> /dev/null
	then
		echo "  Ok, we can continue"
	else
		echo "  Failed, Neo has no Internet access, or server is down"
		echo "  Read http://wiki.openmoko.org/wiki/Getting_Started_with_your_Neo_FreeRunner/cs#Allow_FreeRunner_to_Connect_to_Internet_via_USB"
		exit 1
fi

echo
echo "Press RETURN to continue, when you're ready..."
read NULL

echo
mkdir $ACCELS_DIR
echo "Working in '$ACCELS_DIR'"

echo
echo "First gesture: RIGHT"
echo "Prepare, we're starting in 10 seconds..."
sleep 10s
./gesm --wii --config=$ACCELS_DIR --record=left.accel --no-header

echo
echo "Second gesture: LEFT"
echo "We're starting again in 5 seconds..."
sleep 5s
./gesm --wii --config=$ACCELS_DIR --record=right.accel --no-header

echo
echo "Pausing for another 5 seconds..."
sleep 5s

