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

#include <driver/device.h>

static struct kset *bus_kset;

int bus_register(struct bus_type *bus)
{
	int retval;

	bus->kobj.kset = bus_kset;

	retval = kobject_add(&bus->kobj);
	if (retval)
		goto out;

	bus->devices_kset = kset_create_and_add("devices", &bus->kobj);
	if (!bus->devices_kset) {
		retval = -1;
		goto bus_devices_fail;
	}

	bus->drivers_kset = kset_create_and_add("drivers", &bus->kobj);
	if (!bus->drivers_kset) {
		retval = -1;
		goto bus_drivers_fail;
	}

	sema_init(&bus->lock, 1);

bus_drivers_fail:
	kset_unregister(bus->devices_kset);
	kset_release(&bus->devices_kset->kobj);
bus_devices_fail:
	kobject_del(&bus->kobj);
out:
	return retval;
}

void bus_unregister(struct bus_type *bus)
{
	kset_unregister(bus->drivers_kset);
	kset_release(&bus->drivers_kset->kobj);
	kset_unregister(bus->devices_kset);
	kset_release(&bus->drivers_kset->kobj);
	kobject_del(&bus->kobj);
}

int bus_for_each_dev(struct bus_type *bus, void *data, int (*fn)(struct device *, void *))
{
	struct kset *dev_kset = bus->devices_kset;
	struct kobject *kobj;
	struct device *dev;

	if (!bus)
		return -1;

	down(&bus->lock);
	list_for_each_entry(kobj, &dev_kset->list, entry) {
		dev = kobj_to_dev(kobj);
		fn(dev, data);
	}
	up(&bus->lock);
	
	return 0;
}

struct device *bus_find_device(struct bus_type *bus,
			       void *data, int (*match)(struct device *dev, void *data))
{
	struct kset *dev_kset = bus->devices_kset;
	struct kobject *kobj;
	struct device *dev;

	if (!bus)
		return NULL;

	down(&bus->lock);
	list_for_each_entry(kobj, &dev_kset->list, entry) {
		dev = kobj_to_dev(kobj);
		if (match(dev, data))
			break;
		else
			dev = NULL;
	}
	up(&bus->lock);
	
	return dev;
}

static bool streq(const char *s1, const char *s2)
{
	while (*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}

	if (*s1 == *s2)
		return TRUE;
	if (!*s1 && *s2 == '\n' && !s2[1])
		return TRUE;
	if (*s1 == '\n' && !s1[1] && !*s2)
		return TRUE;
	return FALSE;
}

static int match_name(struct device *dev, void *data)
{
	const char *name = data;
	return streq(name, dev_name(dev));
}

struct device *bus_find_device_by_name(struct bus_type *bus,
	const char *name)
{
	return bus_find_device(bus, (void *)name, match_name);
}

int bus_for_each_drv(struct bus_type *bus, void *data, int (*fn)(struct device_driver *, void *))
{
	struct kset *drv_kset = bus->drivers_kset;
	struct kobject *kobj;
	struct device_driver *drv;

	if (!bus)
		return -1;

	down(&bus->lock);
	list_for_each_entry(kobj, &drv_kset->list, entry) {
		drv = kobj_to_drv(kobj);
		fn(drv, data);
	}
	up(&bus->lock);
	
	return 0;
}

int bus_add_device(struct device *dev)
{
	struct bus_type *bus = dev->bus;

	if (bus) {
		dev->kobj.kset = bus->devices_kset;
		kobject_add(&dev->kobj);
		return 0;
	}

	return -1;
}

static int add_dev(struct device_driver *drv, void *data)
{
	struct device *dev = (struct device *)data;
	int ret = 0;

	if (drv->probe)
	{
		ret = drv->probe(dev);
		if (0 == ret) {
			list_add_tail(&dev->driver_entry, &drv->list_devices);
		}
	}

	return ret;
}

void bus_probe_device(struct device *dev)
{
	struct bus_type *bus = dev->bus;

	if (!bus)
		return;

	down(&bus->lock);
	bus_for_each_drv(bus, dev, add_dev);
	up(&bus->lock);
	
	return;
}

static int remove_dev(struct device_driver *drv, void *data)
{
	struct device *dev = (struct device *)data;
	int ret = 0;

	if (drv->remove)
	{
		ret = drv->remove(dev);
		if (0 == ret) {
			list_del(&dev->driver_entry);
		}
	}

	return ret;
}

void bus_remove_device(struct device *dev)
{
	struct bus_type *bus = dev->bus;

	if (!bus)
		return;

	down(&bus->lock);
	bus_for_each_drv(bus, dev, remove_dev);
	kobject_del(&dev->kobj);
	up(&bus->lock);
}

static int add_drv(struct device *dev, void *data)
{
	struct device_driver *drv = (struct device_driver *)data;
	int ret = 0;

	if (drv->probe)
	{
		ret = drv->probe(dev);
		if (0 == ret) {
			list_add_tail(&dev->driver_entry, &drv->list_devices);
		}
	}
	
	return ret;
}

int bus_add_driver(struct device_driver *drv)
{
	struct bus_type *bus = drv->bus;
	int ret = 0;
	
	if (!bus)
		return -1;

	down(&bus->lock);
	drv->kobj.kset = bus->drivers_kset;
	kobject_add(&drv->kobj);

	ret = bus_for_each_dev(bus, drv, add_drv);
	up(&bus->lock);
	
	return ret;
}

void bus_remove_driver(struct device_driver *drv)
{
	struct bus_type *bus = drv->bus;
	struct device *dev = NULL;
	int ret = 0;

	down(&bus->lock);
	kobject_del(&drv->kobj);
	list_for_each_entry(dev, &drv->list_devices, driver_entry) {
		if (drv->remove)
		{
			ret = drv->remove(dev);
			if (0 == ret) {
				list_del(&dev->driver_entry);
			}
		}
	}
	up(&bus->lock);
}

int buses_init(void)
{
	bus_kset = kset_create_and_add("bus", NULL);
	if (!bus_kset)
		return -1;

	return 0;
}
