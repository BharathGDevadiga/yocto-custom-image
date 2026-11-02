@echo off
title Bharath Custom Embedded Linux (QEMU)
echo ========================================================
echo  Launching Bharath Custom Embedded Linux via QEMU...
echo  To exit cleanly: type 'poweroff' inside the Linux shell
echo ========================================================
echo.
wsl -e bash -c "cd /mnt/c/Users/devad/Downloads/yocto-custom-image && ./run-prebuilt.sh"
pause
