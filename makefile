TARGET := pixeline
BUILD_DIR := build/
SOURCE_DIR := src/
INCLUDE_DIRS := /usr/include/x86_64-linux-gnu/qt6 /usr/include/x86_64-linux-gnu/qt6/QtWidgets /usr/include/x86_64-linux-gnu/qt6/QtGui /usr/include/x86_64-linux-gnu/qt6/QtCore /usr/lib/x86_64-linux-gnu/qt6/mkspecs/linux-g++
LIBS := Qt6Widgets Qt6Gui Qt6Core pthread png
CXXFLAGS := --std=c++20 -g3 -Wall -Wextra -fPIC


LDFLAGS := $(CXXFLAGS) $(addprefix -l, $(LIBS))
CXXFLAGS += -MMD -MP -I$(SOURCE_DIR) $(addprefix -I, $(INCLUDE_DIRS))
THIS_MAKEFILE := $(firstword $(MAKEFILE_LIST))
SOURCES != find $(SOURCE_DIR) -name "*.cpp"
OBJECTS := ${SOURCES:$(SOURCE_DIR)%.cpp=$(BUILD_DIR)%.o}
DEPFILES := ${OBJECTS:%.o=%.d}
-include $(DEPFILES)

HEADERS != find $(SOURCE_DIR) -name "*.h"
OBJECTS += ${HEADERS:$(SOURCE_DIR)%.h=$(BUILD_DIR)%.moc}
MOCDEFINES  := -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_CORE_LIB
MOCFLAGS := -pipe -O2 -Wall -Wextra -D_REENTRANT -fPIC $(DEFINES)
MOC := /usr/lib/qt6/libexec/moc

$(BUILD_DIR)%.moc: $(SOURCE_DIR)%.h $(THIS_MAKEFILE)
	@mkdir -p "${@D}"
	@echo "[MOC] ${@F}"
	@$(MOC) $(DEFINES) $(LDFLAGS_REQ) $(INCLUDE_FLAGS) "$<" -o "$@.cpp"
	@$(CXX) $(CXXFLAGS) -c "$@.cpp" -o "$@"

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
