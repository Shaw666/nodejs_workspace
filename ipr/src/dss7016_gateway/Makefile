
CPP			= g++
INC_DIR		= ./include
LIB_DIR		= /opt/dss7016_gateway_lib
TARGET_DIR	= ../dist/apps/dss7016_gateway/bin

CPPFLAGS    = -g -pipe

INCFLAGS	= -I $(INC_DIR)
INCFLAGS   += -I $(INC_DIR)/log4c

LDFLAGS     = -L$(LIB_DIR)/DPSDK_64
LDFLAGS    += -L$(LIB_DIR)/libxml2_64
LDFLAGS    += -L$(LIB_DIR)/things_64
LDFLAGS    += -L$(LIB_DIR)/log4c_64 
LDFLAGS    += -lssl -lcrypto -lDPSDK_Core -lxml2 -lthings -llog4c -lrt

APP_NAME    = dss7016_gateway

all:
	$(CPP) $(CPPFLAGS)  *.cpp -o $(APP_NAME)  $(INCFLAGS) $(LDFLAGS)
	cp log4crc $(TARGET_DIR)
	cp $(APP_NAME) $(TARGET_DIR)

clean:
	rm -f  dss7016_gateway
