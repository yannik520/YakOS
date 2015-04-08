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

extern int kobject_add(struct kobject *kobj);
extern void kobject_del(struct kobject *kobj);
extern struct kobject *kobject_create(void);
extern struct kobject *kobject_create_and_add(const char *name, struct kobject *parent);
extern void kset_init(struct kset *k);
extern int kset_register(struct kset *k);
extern void kset_unregister(struct kset *k);
extern struct kobject *kset_find_obj(struct kset *kset, const char *name);
extern void kset_release(struct kobject *kobj);
extern struct kset *kset_create(const char *name, struct kobject *parent_kobj);
extern struct kset *kset_create_and_add(const char *name, struct kobject *parent_kobj);

#endif
