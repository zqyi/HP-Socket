LOCAL_PATH := $(call my-dir)

# target lib : hp-testecho-pack-server
include $(CLEAR_VARS)
LOCAL_MODULE     := hp-testecho-pack-server
LOCAL_SRC_FILES  := server/server.cpp \
					../global/helper.cpp
LOCAL_WHOLE_STATIC_LIBRARIES := $(MY_WHOLE_STATIC_LIBRARIES) hpsocket
LOCAL_C_INCLUDES := $(MY_C_INCLUDES)
LOCAL_CFLAGS     := $(MY_CFLAGS) -D_NEED_SSL -D_NEED_HTTP
LOCAL_CPPFLAGS   := $(MY_CPPFLAGS)
LOCAL_LDLIBS     := $(MY_LDLIBS)
include $(BUILD_EXECUTABLE)

# target lib :  hp-testecho-pack-client
include $(CLEAR_VARS)
LOCAL_MODULE     := hp-testecho-pack-client
LOCAL_SRC_FILES  := client/client.cpp \
					../global/helper.cpp
LOCAL_WHOLE_STATIC_LIBRARIES := $(MY_WHOLE_STATIC_LIBRARIES) hpsocket
LOCAL_C_INCLUDES := $(MY_C_INCLUDES)
LOCAL_CFLAGS     := $(MY_CFLAGS) -D_NEED_SSL -D_NEED_HTTP
LOCAL_CPPFLAGS   := $(MY_CPPFLAGS)
LOCAL_LDLIBS     := $(MY_LDLIBS)
include $(BUILD_EXECUTABLE)

# target lib :  hp-testecho-pack-agent
include $(CLEAR_VARS)
LOCAL_MODULE     := hp-testecho-pack-agent
LOCAL_SRC_FILES  := agent/agent.cpp \
					../global/helper.cpp
LOCAL_WHOLE_STATIC_LIBRARIES := $(MY_WHOLE_STATIC_LIBRARIES) hpsocket
LOCAL_C_INCLUDES := $(MY_C_INCLUDES)
LOCAL_CFLAGS     := $(MY_CFLAGS) -D_NEED_SSL -D_NEED_HTTP
LOCAL_CPPFLAGS   := $(MY_CPPFLAGS)
LOCAL_LDLIBS     := $(MY_LDLIBS)
include $(BUILD_EXECUTABLE)
