#ifndef IOCTLS_H
#define IOCTLS_H

#ifndef CTL_CODE
   #pragma message("CTL_CODE undefined. Include winioctl.h or wdm.h")
#endif
#define IOCTL_GET_DRIVER_LIST CTL_CODE(      \
              FILE_DEVICE_UNKNOWN,           \
              0x801,                         \
              METHOD_BUFFERED,               \
              FILE_ANY_ACCESS)

#define IOCTL_GET_DEVICE_LIST CTL_CODE(      \
              FILE_DEVICE_UNKNOWN,           \
              0x802,                         \
              METHOD_BUFFERED,               \
              FILE_ANY_ACCESS)

#define IOCTL_STORE_DEVICE CTL_CODE(      \
              FILE_DEVICE_UNKNOWN,           \
              0x803,                         \
              METHOD_BUFFERED,               \
              FILE_ANY_ACCESS)

#define IOCTL_GET_DEVICE_INFO CTL_CODE(      \
              FILE_DEVICE_UNKNOWN,           \
              0x804,                         \
              METHOD_BUFFERED,               \
              FILE_ANY_ACCESS)

#define IOCTL_GET_DIRECT_DEVICE_INFO CTL_CODE(      \
              FILE_DEVICE_UNKNOWN,           \
              0x805,                         \
              METHOD_BUFFERED,               \
              FILE_ANY_ACCESS)
#endif