TARGET := pixeline
BUILD_DIR := build/
SOURCE_DIR := src/
INCLUDE_DIRS := /usr/include/x86_64-linux-gnu/qt6 /usr/include/x86_64-linux-gnu/qt6/QtWidgets /usr/include/x86_64-linux-gnu/qt6/QtGui /usr/include/x86_64-linux-gnu/qt6/QtCore /usr/lib/x86_64-linux-gnu/qt6/mkspecs/linux-g++
LIBS := Qt6Widgets Qt6Gui Qt6Core pthread png
LDFLAGS := $(CXXFLAGS) ${LIBS:%=-l%}
CXXFLAGS := -I$(SOURCE_DIR) ${INCLUDE_DIRS:%=-I%} -MMD -MP --std=c++20 -g3 -Wall -Wextra

THIS_MAKEFILE := $(firstword $(MAKEFILE_LIST))
SOURCES != find $(SOURCE_DIR) -name "*.cpp"
OBJECTS := ${SOURCES:$(SOURCE_DIR)%.cpp=$(BUILD_DIR)%.o}
DEPFILES := ${OBJECTS:%.o=%.d}
-include $(DEPFILES)

$(BUILD_DIR)%.o: $(SOURCE_DIR)%.cpp $(THIS_MAKEFILE)
	@mkdir -p "${@D}"
	@echo "[CXX] ${@F}"
	@$(CXX) $(CXXFLAGS) -c "$<" -o "$@"

$(TARGET): $(OBJECTS)
	@echo "[LD] ${@F}"
	@$(CXX) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

all: $(TARGET)

clean:
	rm -rf $(TARGET)
	rm -rf $(BUILD_DIR)

.PHONY: clean all
