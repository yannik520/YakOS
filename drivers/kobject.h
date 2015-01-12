#ifndef __KOBJECT_H__
#define __KOBJECT_H__

#include <kernel/semaphore.h>

struct kset;

struct kobject {
	const char		*name;
	struct list_head	 entry;
	struct kobject		*parent;
	struct kset		*kset;
};

struct kset {
	struct list_head	list;
	struct semaphore	list_lock;
	struct kobject		kobj;
};

static inline struct kset *to_kset(struct kobject *kobj)
{
	return kobj ? container_of(kobj, struct kset, kobj) : NULL;
}

static inline const char *kobject_name(const struct kobject *kobj)
{
	return kobj->name;
}


#endif
