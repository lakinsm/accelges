#!/bin/sh

export GESM_PATH=../../gesd
export CONFIG_PATH=../../config/wii
export CONFIG_FILE=wii.cfg

# RIGHT

$GESM_PATH/gesd --sim $CONFIG_PATH/right_test_1.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

$GESM_PATH/gesd --sim $CONFIG_PATH/right_test_2.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

$GESM_PATH/gesd --sim $CONFIG_PATH/right_test_3.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

echo "Press RETURN..."
read NULL
# LEFT

$GESM_PATH/gesd --sim $CONFIG_PATH/left_test_1.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

$GESM_PATH/gesd --sim $CONFIG_PATH/left_test_2.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

$GESM_PATH/gesd --sim $CONFIG_PATH/left_test_3.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

echo "Press RETURN..."
read NULL
# UP

$GESM_PATH/gesd --sim $CONFIG_PATH/up_test_1.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

$GESM_PATH/gesd --sim $CONFIG_PATH/up_test_2.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

$GESM_PATH/gesd --sim $CONFIG_PATH/up_test_3.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

echo "Press RETURN..."
read NULL
# DOWN

$GESM_PATH/gesd --sim $CONFIG_PATH/down_test_1.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

$GESM_PATH/gesd --sim $CONFIG_PATH/down_test_2.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

$GESM_PATH/gesd --sim $CONFIG_PATH/down_test_3.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

echo "Press RETURN..."
read NULL
#FORWARD

$GESM_PATH/gesd --sim $CONFIG_PATH/forward_test_1.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

$GESM_PATH/gesd --sim $CONFIG_PATH/forward_test_2.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

$GESM_PATH/gesd --sim $CONFIG_PATH/forward_test_3.accel --config $CONFIG_PATH/$CONFIG_FILE --no-dbus

