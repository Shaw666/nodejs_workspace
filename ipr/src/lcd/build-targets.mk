# Sources to compile

SRC_FILES := main.cpp
SRC_FILES += lcd_driver.cpp
SRC_FILES += ascii_8_16.cpp
SRC_FILES += ascii_8_8.cpp
SRC_FILES += chars.cpp
SRC_FILES += conwin_logo.cpp


OBJ_FILES = $(addprefix $(BUILD_PATH)/,$(SRC_FILES:.cpp=.o))

$(BUILD_PATH)/%.o: user/%.cpp
	$(SILENT_CXX) $(CXX) $(CFLAGS) $(CXXFLAGS) $(LIBMAPLE_INCLUDES) $(WIRISH_INCLUDES) -c -o $@  $< 

$(BUILD_PATH)/libmaple.a: $(BUILDDIRS) $(TGT_BIN)
	- rm -f $@
	$(AR) crv $(BUILD_PATH)/libmaple.a $(TGT_BIN)

library: $(BUILD_PATH)/libmaple.a

.PHONY: library

$(BUILD_PATH)/$(BOARD).elf: $(BUILDDIRS) $(TGT_BIN) $(OBJ_FILES)
	$(SILENT_LD) $(CXX) $(LDFLAGS) -o $@ $(TGT_BIN) $(OBJ_FILES) -Wl,-Map,$(BUILD_PATH)/$(BOARD).map

$(BUILD_PATH)/$(BOARD).hex: $(BUILD_PATH)/$(BOARD).elf
	$(SILENT_OBJCOPY) $(OBJCOPY) -v -Oihex $(BUILD_PATH)/$(BOARD).elf $@ 1>/dev/null

$(BUILD_PATH)/$(BOARD).bin: $(BUILD_PATH)/$(BOARD).elf
	$(SILENT_OBJCOPY) $(OBJCOPY) -v -Obinary $(BUILD_PATH)/$(BOARD).elf $@ 1>/dev/null
	$(SILENT_DISAS) $(DISAS) -d $(BUILD_PATH)/$(BOARD).elf > $(BUILD_PATH)/$(BOARD).disas
	@echo " "
	@echo "Object file sizes:"
	@find $(BUILD_PATH) -iname *.o | xargs $(SIZE) -t > $(BUILD_PATH)/$(BOARD).sizes
	@cat $(BUILD_PATH)/$(BOARD).sizes
	@echo " "
	@echo "Final Size:"
	@$(SIZE) $<
	@echo $(MEMORY_TARGET) > $(BUILD_PATH)/build-type

$(BUILDDIRS):
	@mkdir -p $@

MSG_INFO:
	@echo "================================================================================"
	@echo ""
	@echo "  Build info:"
	@echo "     BOARD:          " $(BOARD)
	@echo "     MCU:            " $(MCU)
	@echo "     MEMORY_TARGET:  " $(MEMORY_TARGET)
	@echo ""
	@echo "  See 'make help' for all possible targets"
	@echo ""
	@echo "================================================================================"
	@echo ""
