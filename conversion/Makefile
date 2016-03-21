#### Compiler and tool definitions shared by all build targets #####
CC = gcc
BASICOPTS = -O3
CFLAGS = $(BASICOPTS)


# Define the target directories.
BUILD_DIR=../build

all: $(BUILD_DIR)/gconv

# Create the target directory (if needed)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


$(BUILD_DIR)/gconv: $(BUILD_DIR) gconv.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ gconv.c


#### Clean target deletes all generated files ####
clean:
	rm -f $(BUILD_DIR)/gconv
	rm -f -r $(BUILD_DIR)
