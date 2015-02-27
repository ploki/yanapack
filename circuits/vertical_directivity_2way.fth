. distance                                 TO listner_distance
. vertical_angle                           TO listner_angle
. listner_distance listner_angle I * EXP * TO listner_position
. distance_between_drivers 2 / I *         TO tweeter_position
. tweeter_position -1 *                    TO woofer_position
. listner_position tweeter_position - DUP
.                             ABS          TO tweeter_distance
.                             ARG          TO tweeter_theta
. listner_position woofer_position  - DUP
.                             ABS          TO woofer_distance
.                             ARG          TO woofer_theta
. tweeter_Sd tweeter_distance tweeter_theta DIRIMP TO za_tweeter
. woofer_Sd   woofer_distance woofer_theta  DIRIMP TO za_woofer
.
. F .
. za_woofer woofer_volume_velocity  *
. za_tweeter tweeter_volume_velocity * + DBSPL .
. za_woofer woofer_volume_velocity  *   DBSPL .
. za_tweeter tweeter_volume_velocity *   DBSPL .
