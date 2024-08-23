#!/bin/bash

RED='\033[1;31m'
GREEN='\033[1;32m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
NC='\033[0m'

function error {
    printf "${RED}ERROR ${NC}$@\n"
}

function warning {
    printf "${YELLOW}WARNING ${NC}$@\n"
}

function success {
    printf "${GREEN}OK ${NC}$@\n"
}

function title {
	printf "${BOLD}$1: ${NC}$2\n" 
}

function highlight {
	printf "${BOLD}$@${NC}\n"
}

function ok_or_return {
    if [ $? -eq 0 ]; then
        echo $(success "$1")
    else
        echo $(error "$2")
        return $?
    fi
}

function ok_or_exit {
    if [ $? -eq 0 ]; then
        echo $(success "$1")
    else
        echo $(error "$2")
        exit $?
    fi
}

function disk_ops_attach {
	if [[ -v EMBEDULAR_DISK_DEVICE ]]; then
		echo $(error "Disk currenty attached at loop device $EMBEDULAR_DISK_DEVICE")
		return 1
	fi

	printf "Querying a free loop device to access image and partitions ... "
	EMBEDULAR_DISK_DEVICE="$(sudo losetup -f)"
	(ok_or_return "$EMBEDULAR_DISK_DEVICE")

	printf "Setting up $EMBEDULAR_DISK_DEVICE to access disk.bin and its partitions ... "
	sudo losetup -P "$EMBEDULAR_DISK_DEVICE" disk.bin
	(ok_or_return)

	# A disk.bin image should have two partitions
	EMBEDULAR_DISK_DEVICE_FAT32="${EMBEDULAR_DISK_DEVICE}p1"
	EMBEDULAR_DISK_DEVICE_RAW="${EMBEDULAR_DISK_DEVICE}p2"

	# Check if both partitions have been exposed
	printf "Checking access to FAT32 partition at $EMBEDULAR_DISK_DEVICE_FAT32 ... "
	( [ -e "$EMBEDULAR_DISK_DEVICE_FAT32" ] && echo $(success) ) || { echo $(error); return 1; }

	printf "Checking access to RAW partition at $EMBEDULAR_DISK_DEVICE_RAW ... "
	( [ -e "$EMBEDULAR_DISK_DEVICE_RAW" ] && echo $(success) ) || { echo $(error); return 1; }

	echo $(highlight "Disk attached")
}

function disk_ops_detach() {
	if [[ ! -v EMBEDULAR_DISK_DEVICE ]]; then
		echo $(error "Disk is not attached")
		return 1
	fi

	printf "Detaching the loop device $EMBEDULAR_DISK_DEVICE ... "
	sudo losetup -d "$EMBEDULAR_DISK_DEVICE"
	(ok_or_return)

	unset EMBEDULAR_DISK_DEVICE
	unset EMBEDULAR_DISK_DEVICE_FAT32
	unset EMBEDULAR_DISK_DEVICE_RAW

	echo $(highlight "Disk detached")
}

function disk_ops_mount() {
	if [[ ! -v LIB_EMBEDULAR_BASE ]]; then
		echo $(error "LIB_EMBEDULAR_BASE not defined")
		return 1
	fi

	if [[ -v EMBEDULAR_DISK_MOUNT ]]; then
		echo $(error "Disk already mounted at $EMBEDULAR_DISK_MOUNT")
		return 1
	fi

	EMBEDULAR_DISK_MOUNT=$LIB_EMBEDULAR_BASE/embedul.ar/disk

	if grep -qs "${EMBEDULAR_DISK_MOUNT} " /proc/mounts; then
		echo $(error "Mount point $EMBEDULAR_DISK_MOUNT already exists in /proc/mounts")
		unset EMBEDULAR_DISK_MOUNT
		return 1
	fi

	if [[ ! -v EMBEDULAR_DISK_DEVICE_FAT32 ]]; then
		disk_ops_attach || { unset EMBEDULAR_DISK_MOUNT; return 1; }
	fi

	mkdir -p "$EMBEDULAR_DISK_MOUNT" || { echo $(error "Cannot create mount point at $EMBEDULAR_DISK_MOUNT"); unset EMBEDULAR_DISK_MOUNT; disk_ops_detach; return 1; }

	printf "Mounting FAT32 disk partition $EMBEDULAR_DISK_DEVICE_FAT32 to $EMBEDULAR_DISK_MOUNT ... "
	sudo mount -t vfat "$EMBEDULAR_DISK_DEVICE_FAT32" "$EMBEDULAR_DISK_MOUNT" -o rw,,uid=$(id -u),gid=$(id -g) || { echo $(error); unset EMBEDULAR_DISK_MOUNT; disk_ops_detach; return 1; }
	echo $(success)

	echo $(highlight "Disk mounted")
}

function disk_ops_unmount() {
	if [[ ! -v EMBEDULAR_DISK_MOUNT ]]; then
		echo $(error "Disk not mounted")
		return 1
	fi

	printf "Unmounting FAT32 disk partition at $EMBEDULAR_DISK_MOUNT ... "
	sudo umount "$EMBEDULAR_DISK_MOUNT"
	(ok_or_return)

	unset EMBEDULAR_DISK_MOUNT
	echo $(highlight "Disk unmounted")

	disk_ops_detach || return 1
}
