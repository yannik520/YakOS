#include <driver/device.h>

static struct kset bus_kset;

int bus_register(struct bus_type *bus)
{
	int retval;

	bus->kobj.kset = &bus_kset;

	retval = kobject_add(&bus->kobj);
	if (retval)
		goto out;

	bus->devices_kset = kset_create_and_add("devices", &bus->kobj);
	if (!bus->deivces_kset) {
		retval = -1;
		goto bus_devices_fail;
	}

	bus->drivers_kset = kset_create_and_add("drivers", &bus->kobj);
	if (!bus->drivers_kset) {
		retval = -1;
		goto bus_drivers_fail;
	}

bus_drivers_fail:
	kset_unregister(bus->devices_kset);
	kset_release(&bus->devices_kset.kobj);
bus_deivces_fail:
	kobject_del(&bus->kobj);
out:
	return retval;
}

void bus_unregister(struct bus_type *bus)
{
	kset_unregister(bus->drivers_kset);
	kset_release(&bus->drivers_kset.kobj);
	kset_unregister(bus->devices_kset);
	kset_release(&bus->drivers_kset.kobj);
	kobject_del(&bus->kobj);
}

int bus_for_each_dev(struct bus_type *bus, void *data, int (*fn)(struct device *, void *))
{
	struct kset *dev_kset = bus->devices_kset;
	struct kobject *kobj;
	struct devices *dev;

	if (!bus)
		return -1;

	list_for_each_entry(kobj, &dev_kset->list, entry) {
		dev = kobj_to_dev(kobj);
		fn(dev, data);
	}

	return 0;
}

struct device *bus_find_device(struct bus_type *bus,
			       void *data, int (*match)(struct device *dev, void *data))
{
	struct kset *dev_kset = bus->devices_kset;
	struct kobject *kobj;
	struct devices *dev;

	if (!bus)
		return NULL;

	list_for_each_entry(kobj, &dev_kset->list, entry) {
		dev = kobj_to_dev(kobj);
		if (match(dev, data))
			break;
		else
			dev = NULL;
	}

	return dev;
}

bool streq(const char *s1, const char *s2)
{
	while (*s1 && *s1 == *s2) {
		s1++;
		s2++;
	}

	if (*s1 == *s2)
		return true;
	if (!*s1 && *s2 == '\n' && !s2[1])
		return true;
	if (*s1 == '\n' && !s1[1] && !*s2)
		return true;
	return false;
}

int match_name(struct device *dev, void *data)
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
	struct devices *drv;

	if (!bus)
		return -1;

	list_for_each_entry(kobj, &drv_kset->list, entry) {
		drv = kobj_to_drv(kobj);
		fn(drv, data);
	}

	return 0;
}
