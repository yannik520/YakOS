#ifndef __DEVICE_H__
#define __DEVICE_H__

#include <driver/kobject.h>

struct device {
	struct device		*parent;
	struct kobject		 kobj;
	struct device_driver	*driver;
	struct but_type		*bus;
	const char		*name;
};

struct device_driver {
	const char	*name;
	struct bus_type *bus;
	struct kobject	 kobj;

	int (*probe) (struct device *dev);
	int (*remove) (struct device *dev);
};

struct bus_type {
	const char		*name;
	struct kobject		 kobj;
	struct kset		 *devices_kset;
	struct kset		 *drivers_kset;

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

extern int bus_register(struct bus_type *bus);

extern void bus_unregister(struct bus_type *bus);

extern int bus_rescan_devices(struct bus_type *bus);
