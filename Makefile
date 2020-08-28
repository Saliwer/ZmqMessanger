THIS_DIR	= $(shell pwd)
INC_DIR		= $(THIS_DIR)/include
SRC_DIR		= $(THIS_DIR)/source
OBJ_DIR		= $(THIS_DIR)/obj

CXX		= @g++

DEBUGFLAG	= -g

STDLIBS		= -lzmq
INCFLAGS	= -I. -I$(INC_DIR)

CXXFLAGS	= -Wall -std=c++11 -Wno-unused-label $(INCFLAGS) $(STDLIBS)

ALL_HEADERS	= $(wildcard $(INC_DIR)/*.h)
ALL_SOURCES	= $(wildcard $(SRC_DIR)/*.cpp)
ALL_OBJECTS	= $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(ALL_SOURCES))

compile: obj $(ALL_OBJECTS)

$(SRC_DIR)/depend.mk: $(HEADERS)
	@echo "Create dependencies file"
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/*.cpp -MM > $(SRC_DIR)/depend.mk

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/depend.mk
	@echo "Compilation: "$<
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "$@ done..."
obj:
	@mkdir -p $(OBJ_DIR)

clean:
	@echo "Cleaning..."
	@rm -f $(OBJ_DIR)/* $(SRC_DIR)/depend.mk ./*~ $(SRC_DIR)/*~ $(INC_DIR)/*~
	@rmdir $(OBJ_DIR)
