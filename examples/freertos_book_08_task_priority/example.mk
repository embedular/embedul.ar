$(call emb_need_var,LIB_EMBEDULAR_ROOT)

BUILD_LIBS += 3rd_party/freertos

CFLAGS += -I$(LIB_EMBEDULAR_ROOT)/examples/freertos_book
APP_OBJS += $(LIB_EMBEDULAR_ROOT)/examples/freertos_book/supporting_functions.o

CHIP_lpc4337jbd144_CFLAGS += -DBSS_SECTION_OSWRAP_FREERTOS_RUN_TASK_STACK='CC_Section(".bss.$$$$RamLoc40")'
CHIP_lpc4337jbd144_CFLAGS += -DBSS_SECTION_OSWRAP_FREERTOS_MAIN_TASK_STACK='CC_Section(".bss.$$$$RamLoc40")'
