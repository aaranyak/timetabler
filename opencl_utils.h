/* Header file for opencl_utis.h */
#ifndef OPENCL_UTILS_H
#define OPENCL_UTILS_H
#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>
char *load_source_string(char *path);
cl_device_id get_usable_gpu();
#endif
