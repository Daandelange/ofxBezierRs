meta:
	ADDON_NAME = ofxBezierRs
	ADDON_DESCRIPTION = OF wrapper for lib bezier-rs, providing a set geometrical functions on cartesian planar Beziér paths or shapes.
	ADDON_AUTHOR = Daan de Lange
	ADDON_TAGS = "Geometry" "Algorithms" "Animation"
	ADDON_URL = https://github.com/daandelange/ofxBezierRs

common:
	ADDON_INCLUDES = src/ 
	ADDON_INCLUDES += libs/bezier-rs-ffi/include
	ADDON_LDFLAGS = -lpthread -ldl

osx:
	ADDON_LIBS = libs/bezier-rs-ffi/lib/osx/libbezier_rs_ffi.dylib

linux64:
	#ADDON_LDFLAGS = -lbezier_rs_ffi
    ADDON_LIBS = libs/bezier-rs-ffi/lib/linux64/libbezier_rs_ffi.so

linuxarmv7l:

	#ADDON_LIBS += libs/bezier-rs-ffi/lib/linuxarmv7l/bezierrs.a

vs:
	#ADDON_LIBS = libs/bezier-rs-ffi/lib/xxxx/libbezier_rs_ffi.dll

