THIS_DIR	= $(shell pwd)
INC_DIR		= $(THIS_DIR)/include
SRC_DIR		= $(THIS_DIR)/source
OBJ_DIR		= $(THIS_DIR)/obj
LIB_DIR		= $(THIS_DIR)/lib

CXX		= @g++

DEBUGFLAG	= -g

STDLIBS		= -lpthread -lzmq
INCFLAGS	= -I. -I$(INC_DIR)

CXXFLAGS	= $(DEBUGFLAG) -Wall -std=c++11 -fPIC $(INCFLAGS) $(STDLIBS)

ALL_HEADERS	= $(wildcard $(INC_DIR)/*.h)
ALL_SOURCES	= $(wildcard $(SRC_DIR)/*.cpp)
ALL_OBJECTS	= $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(ALL_SOURCES))

compile: obj lib $(ALL_OBJECTS) $(LIB_DIR)/ZmqMsgLib

$(SRC_DIR)/depend.mk: $(HEADERS)
	@echo "Create dependencies file"
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/*.cpp -MM > $(SRC_DIR)/depend.mk

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(SRC_DIR)/depend.mk
	@echo "Compilation: "$<
	$(CXX) $(CXXFLAGS) -c $< -o $@
	@echo "$@ done..."
obj:
	@mkdir -p $(OBJ_DIR)
lib:
	@mkdir -p $(LIB_DIR)

$(LIB_DIR)/ZmqMsgLib: $(LIB_DIR)/libZmqMsg.so $(LIB_DIR)/libZmqMsg.a
	@echo "$@ done!"

$(LIB_DIR)/libZmqMsg.so: $(ALL_OBJECTS)
	$(CXX) -shared -o $@ $^
	@echo "$@ done!"

$(LIB_DIR)/libZmqMsg.a: $(ALL_OBJECTS)
	@ar rcs $@ $^
	@echo "$@ done!"

clean:
	@echo "Cleaning..."
	@rm -f $(OBJ_DIR)/* $(LIB_DIR)/* $(SRC_DIR)/depend.mk ./*~ $(SRC_DIR)/*~ $(INC_DIR)/*~
