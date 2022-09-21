# shellcheck shell=sh

# Add Arm GNU toolchain to $PATH
arm_toolchain_path=($(echo "/opt/arm-gnu-toolchain-*/bin"))
if [ -n "${arm_toolchain_path}" ] && [ -n "${PATH##*${arm_toolchain_path}}" ] && [ -n "${PATH##*${arm_toolchain_path}:*}" ]; then
    export PATH=$PATH:${arm_toolchain_path}
fi
