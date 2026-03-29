#ifndef _HBP_IOCTL_H_
#define _HBP_IOCTL_H_

#define HBP_IOCTL_MAGIC  'h'

#define CMD_SET_MODE     _IOW(HBP_IOCTL_MAGIC, 1, int)
#define CMD_ENABLE       _IO(HBP_IOCTL_MAGIC, 2)
#define CMD_DISABLE      _IO(HBP_IOCTL_MAGIC, 3)

#endif