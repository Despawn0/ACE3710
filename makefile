# compiler settings
INC := AssemblerLibs
CFLAGS_ := $(CFLAGS) -I$(INC) -O2

# targets
EXEC := ace3710
BUILD_DIR := ./build
SRCS := $(shell find $(./) -name '*.c')
OBJS := $(SRCS:./%.c=$(BUILD_DIR)/%.o)

# for errors
.DELETE_ON_ERROR:

# build executable
$(EXEC): $(OBJS)
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@rm -rf $(BUILD_DIR)

# build objects
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS_) -c $< -o $@ $(LDFLAGS)

# clean
.PHONY: clean
clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(EXEC)

# I don't do much with makefiles
# This should work, used https://makefiletutorial.com/#static-pattern-rules as a source