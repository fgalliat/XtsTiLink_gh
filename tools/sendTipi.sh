export rpi="192.168.1.16"
#export rpi="192.168.43.156"

scp /vm_mnt/devl/TI_92/Arduino/XtsTiLink/devl4ti/test_gcc4ti_1/xtsterm.92p root@$rpi:/vm_mnt/XtsTiLink/devl4ti/test_gcc4ti_1/
scp /vm_mnt/devl/TI_92/Arduino/XtsTiLink/tools/welcm.sh root@$rpi:/vm_mnt/XtsTiLink/tools/

scp /vm_mnt/devl/TI_92/Arduino/XtsTiLink/tools/src/SyncArduinoPipe.java root@$rpi:/vm_mnt/XtsTiLink/tools/src/
scp /vm_mnt/devl/TI_92/Arduino/XtsTiLink/tools/src/Shell.java root@$rpi:/vm_mnt/XtsTiLink/tools/src/