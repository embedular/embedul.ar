# shellcheck shell=sh

# Add Arm GNU toolchain to $PATH
arm_toolchain_path=($(echo "/opt/*arm-none-eabi*/bin"))
if [ -n "${arm_toolchain_path}" ] && [ -n "${PATH##*${arm_toolchain_path}}" ] && [ -n "${PATH##*${arm_toolchain_path}:*}" ]; then
    export PATH=$PATH:${arm_toolchain_path}
fi
