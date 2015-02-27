! freq
. F .
! -1 cycle and 1 cycle
. 1 F / 1000 * .
.
. _F F - 2 PI * * 1000 /
. DUP
! group delay of the midbass driver
.   _woofer_volume_velocity  ARG
.    woofer_volume_velocity  ARG ANGLE
.                                SWAP / NEG .
! group delay of the HF driver
.   _tweeter_volume_velocity ARG
.    tweeter_volume_velocity ARG ANGLE
.                                SWAP / NEG .

