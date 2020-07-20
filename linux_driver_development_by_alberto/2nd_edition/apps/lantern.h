/*
 * Copyright (C) 2018 Microchip Technology Inc.
 * Marek Sieranski <marek.sieranski@microchip.com>
 *
 * lantern.h - common definitions for the lantern driver and the application.
 *
 * This program is free software, you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your discretion) any later version.
 */
#ifndef __LANTERN_H
#define __LANTERN_H

#define LANTERN_IOC_MAGIC    'n'
#define LANTERN_IOC_CLEAR    _IO(LANTERN_IOC_MAGIC,90)
#define LANTERN_IOC_READ     _IOR(LANTERN_IOC_MAGIC,91,int)
#define LANTERN_IOC_WRITE    _IOW(LANTERN_IOC_MAGIC,92,int)
#define LANTERN_IOC_TOGGLE   _IO(LANTERN_IOC_MAGIC,93)

#endif /* __LANTERN_H */
