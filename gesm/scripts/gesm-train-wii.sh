#!/bin/sh

export GESM_PATH=../../gesm
export CONFIG_PATH=../../config/wii

# RIGHT

$GESM_PATH/gesm --sim $CONFIG_PATH/right_train_1.accel --config $CONFIG_PATH --new right.model

$GESM_PATH/gesm --sim $CONFIG_PATH/right_train_2.accel --config $CONFIG_PATH --train right.model

$GESM_PATH/gesm --sim $CONFIG_PATH/right_train_3.accel --config $CONFIG_PATH --train right.model

$GESM_PATH/gesm --sim $CONFIG_PATH/right_train_4.accel --config $CONFIG_PATH --train right.model

$GESM_PATH/gesm --sim $CONFIG_PATH/right_train_5.accel --config $CONFIG_PATH --train right.model

# LEFT

$GESM_PATH/gesm --sim $CONFIG_PATH/left_train_1.accel --config $CONFIG_PATH --new left.model

$GESM_PATH/gesm --sim $CONFIG_PATH/left_train_2.accel --config $CONFIG_PATH --train left.model

$GESM_PATH/gesm --sim $CONFIG_PATH/left_train_3.accel --config $CONFIG_PATH --train left.model

$GESM_PATH/gesm --sim $CONFIG_PATH/left_train_4.accel --config $CONFIG_PATH --train left.model

$GESM_PATH/gesm --sim $CONFIG_PATH/left_train_5.accel --config $CONFIG_PATH --train left.model

# UP

$GESM_PATH/gesm --sim $CONFIG_PATH/up_train_1.accel --config $CONFIG_PATH --new up.model

$GESM_PATH/gesm --sim $CONFIG_PATH/up_train_2.accel --config $CONFIG_PATH --train up.model

$GESM_PATH/gesm --sim $CONFIG_PATH/up_train_3.accel --config $CONFIG_PATH --train up.model

$GESM_PATH/gesm --sim $CONFIG_PATH/up_train_4.accel --config $CONFIG_PATH --train up.model

$GESM_PATH/gesm --sim $CONFIG_PATH/up_train_5.accel --config $CONFIG_PATH --train up.model

# DOWN

$GESM_PATH/gesm --sim $CONFIG_PATH/down_train_1.accel --config $CONFIG_PATH --new down.model

$GESM_PATH/gesm --sim $CONFIG_PATH/down_train_2.accel --config $CONFIG_PATH --train down.model

$GESM_PATH/gesm --sim $CONFIG_PATH/down_train_3.accel --config $CONFIG_PATH --train down.model

$GESM_PATH/gesm --sim $CONFIG_PATH/down_train_4.accel --config $CONFIG_PATH --train down.model

$GESM_PATH/gesm --sim $CONFIG_PATH/down_train_5.accel --config $CONFIG_PATH --train down.model

#FORWARD

$GESM_PATH/gesm --sim $CONFIG_PATH/forward_train_1.accel --config $CONFIG_PATH --new forward.model

$GESM_PATH/gesm --sim $CONFIG_PATH/forward_train_2.accel --config $CONFIG_PATH --train forward.model

$GESM_PATH/gesm --sim $CONFIG_PATH/forward_train_3.accel --config $CONFIG_PATH --train forward.model

$GESM_PATH/gesm --sim $CONFIG_PATH/forward_train_4.accel --config $CONFIG_PATH --train forward.model

$GESM_PATH/gesm --sim $CONFIG_PATH/forward_train_5.accel --config $CONFIG_PATH --train forward.model

