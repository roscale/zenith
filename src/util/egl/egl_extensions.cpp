#include "egl_extensions.hpp"

PFNEGLCREATESYNCKHRPROC eglCreateSyncKHR;
PFNEGLDESTROYSYNCKHRPROC eglDestroySyncKHR;
PFNEGLDUPNATIVEFENCEFDANDROIDPROC eglDupNativeFenceFDANDROID;

void load_egl_extensions() {
	eglCreateSyncKHR = reinterpret_cast<PFNEGLCREATESYNCKHRPROC>(
		  eglGetProcAddress("eglCreateSyncKHR"));
	eglDestroySyncKHR = reinterpret_cast<PFNEGLDESTROYSYNCKHRPROC>(
		  eglGetProcAddress("eglDestroySyncKHR"));
	eglDupNativeFenceFDANDROID = reinterpret_cast<PFNEGLDUPNATIVEFENCEFDANDROIDPROC>(
		  eglGetProcAddress("eglDupNativeFenceFDANDROID"));
}
