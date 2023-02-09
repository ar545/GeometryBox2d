########################
#
# Main Application Entry
#
########################
LOCAL_PATH := $(call my-dir)
CUGL_PATH  := $(LOCAL_PATH)/../../../../../../../cugl
PROJ_PATH  := $(LOCAL_PATH)/../../../../../..
include $(CLEAR_VARS)

LOCAL_MODULE := main
LOCAL_C_INCLUDES := $(CUGL_PATH)/include
LOCAL_C_INCLUDES += $(CUGL_PATH)/sdl2/include
LOCAL_C_INCLUDES += $(CUGL_PATH)/thirdparty/box2d/include
LOCAL_C_INCLUDES += $(CUGL_PATH)/thirdparty/geomtools/include
LOCAL_C_INCLUDES += $(PROJ_PATH)/source


# Add your application source files here.
LOCAL_PATH = $(PROJ_PATH)
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/source/GLApp.cpp \
	$(LOCAL_PATH)/source/GLGameScene.cpp \
	$(LOCAL_PATH)/source/GLInputController.cpp \
	$(LOCAL_PATH)/source/GLLoadingScene.cpp \
	$(LOCAL_PATH)/source/GLStar.cpp \
	$(LOCAL_PATH)/source/main.cpp)

# Link in SDL2
LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_image
LOCAL_SHARED_LIBRARIES += SDL2_ttf
LOCAL_SHARED_LIBRARIES += SDL2_codec

LOCAL_STATIC_LIBRARIES := CUGL
LOCAL_STATIC_LIBRARIES += box2d
LOCAL_STATIC_LIBRARIES += poly2tri
LOCAL_STATIC_LIBRARIES += polyclipping

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lGLESv3 -lOpenSLES -llog -landroid
include $(BUILD_SHARED_LIBRARY)
