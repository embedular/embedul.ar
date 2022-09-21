include $(LIB_EMBEDULAR_PATH)/embedul.ar/makefiles/system.mk

# To build a default example from the command-line with default parameters
EXAMPLE ?= log_hello_world
BUILD_TARGET ?= native_hosted

$(call emb_need_var,EXAMPLE)
$(call emb_need_dir,./examples/$(EXAMPLE),'$(EXAMPLE)' example)
$(call emb_info,Compiling example '$(EXAMPLE)')

-include ./examples/$(EXAMPLE)/config.mk

APP_OBJS += \
    ./examples/$(EXAMPLE)/main.o

$(BUILD)
