include $(realpath ./makefiles/system.mk)

# To build a default example from the command-line with default parameters
EXAMPLE ?= hello_world
BUILD_TARGET ?= native_hosted

$(call emb_need_var,EXAMPLE)
$(call emb_need_dir,./examples/$(EXAMPLE),'$(EXAMPLE)' example)
$(call emb_info,Compiling example '$(EXAMPLE)')

APP_OBJS += \
    ./examples/$(EXAMPLE)/main.o

# (Optional) Specific example makefile params
-include ./examples/$(EXAMPLE)/example.mk

$(BUILD)
