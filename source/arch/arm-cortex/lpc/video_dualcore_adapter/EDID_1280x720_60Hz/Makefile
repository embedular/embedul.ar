include $(LIB_EMBEDULAR_BASE)/embedul.ar/makefiles/system.mk

VIDEO_ADAPTER_MODELINE := EDID_1280x720_60Hz
VIDEO_ADAPTER_LINEOUT := fb256-17239ns
# Required hsync frequency of 45000 Hz, measured 44994 Hz
# Tune at -1 gives 45003 Hz
VIDEO_ADAPTER_HSYNC_RIT_TUNE ?= -1
# Required 404 ns, measured 482 ns (-78 ns, ~16 instructions at 204 Mhz).
VIDEO_ADAPTER_HSYNC_IC_TUNE ?= -17
# Required 3097 ns, measured 3520 ns (-423 ns, ~86 instructions at 204 Mhz).
VIDEO_ADAPTER_BACK_PORCH_IC_TUNE ?= -89
# Required vsync pulse frequency 60 Hz, measured: 60.0051 Hz

$(BUILD)
