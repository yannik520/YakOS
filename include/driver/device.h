/*
 * Copyright (c) 2013 Yannik Li(Yanqing Li)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <driver/kobject.h>

struct device {
	struct device		*parent;
	struct kobject		 kobj;
	struct device_driver	*driver;
	struct list_head	driver_entry;
	struct bus_type		*bus;
	const char		*name;
};

struct device_driver {
	const char	*name;
	struct bus_type *bus;
	struct kobject	 kobj;
	struct list_head list_devices;

	int (*probe) (struct device *dev);
	int (*remove) (struct device *dev);
};

struct bus_type {
	const char		*name;
	struct kobject		 kobj;
	struct kset		*devices_kset;
	struct kset		*drivers_kset;
	struct semaphore	 lock;

	int (*match)(struct device *dev, struct device_driver *drv);
	int (*probe)(struct device *dev);
	int (*remove)(struct device *dev);
};

static inline struct device *kobj_to_dev(struct kobject *kobj)
{
	return container_of(kobj, struct device, kobj);
}

static inline struct device_driver *kobj_to_drv(struct kobject *kobj)
{
	return container_of(kobj, struct device_driver, kobj);
}

static inline const char *dev_name(const struct device *dev)
{
	return kobject_name(&dev->kobj);
}

extern int	bus_register(struct bus_type *bus);
extern void	bus_unregister(struct bus_type *bus);
extern int	bus_add_device(struct device *dev);
extern void	bus_probe_device(struct device *dev);
extern void	bus_remove_device(struct device *dev);
extern int	bus_add_driver(struct device_driver *drv);
extern void	bus_remove_driver(struct device_driver *drv);
extern int	buses_init(void);

#endif
