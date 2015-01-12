#include <kernel/printk.h>
#include <kernel/list.h>
#include <mm/malloc.h>
#include <driver/device.h>
#include <string.h>

static void kobject_init(struct kobject *kobj)
{
	if (!kobj)
		return;

	INIT_LIST_HEAD(&kobj->entry);
}

/* add the kobject to its kset's list */
static void kobj_kset_join(struct kobject *kobj)
{
	if (!kobj->kset)
		return;

	down(&kobj->kset->list_lock);
	list_add_tail(&kobj->entry, &kobj->kset->list);
	up(&kobj->kset->list_lock);
}

static void kobj_kset_leave(struct kobject *kobj)
{
	if (!kobj->kset)
		return;

	down(&kobj->kset->list_lock);
	list_del_init(&kobj->entry);
	up(&kobj->kset->list_lock);
}

int kobject_add(struct kobject *kobj)
{
	struct kobject *parent;

	if (!kobj)
		return -1;

	parent = kobj->parent;

	if (kobj->kset) {
		if (!parent)
			parent = &kobj->kset->kobj;
		
		kobj_kset_join(kobj);
		kobj->parent = parent;
	}

	return 0;
}

void kobject_del(struct kobject *kobj)
{
	if (!kobj)
		return;

	kobj_kset_leave(kobj);
	kobj->parent = NULL;
}

struct kobject *kobject_create(void)
{
	struct kobject *kobj;
	
	kobj = kmalloc(sizeof(*kobj));
	if (!kobj)
		return NULL;

	memset(kobj, 0, sizeof(*kobj));
	kobject_init(kobj);

	reutnr kobj;
}

struct kobject *kobject_create_and_add(const char *name, struct kobject *parent)
{
	struct kobject *kobj;
	int retval;

	kobj = kobject_create();
	if (!kobj)
		return NULL;

	kobj->name = name;
	kobj->parent = parent;
	
	retval = kobject_add(kobj);
	if (retval) {
		printk("%s: kobject_add error: %d\n", __func__, retval);
		kobj = NULL;
	}

	return kobj;
}

void kset_init(struct kset *k)
{
	kobject_init(&k->kobj);
	INIT_LIST_HEAD(&k->list);
	sema_init(&k->list_lock, 1);
}

int kset_register(struct kset *k)
{
	int err;

	if (!k)
		return -1;

	kset_init(k);
	err = kobj_add(&k->kobj);
	if (err)
		return err;

	return 0;
}

void kset_unregister(struct kset *k)
{
	if (!k)
		return;
	
	kobject_del(&k->kobj);
}

struct kobject *kset_find_obj(struct kset *kset, const char *name)
{
	struct kobject *k;
	struct kobject *ret = NULL;

	down(&kset->list_lock);

	list_for_each_entry(k, &kset->list, entry) {
		if (kobject_name(k) && !strcmp(kobject_name(k), name)) {
			ret = k;
			break;
		}
	}

	up(&kset->list_lock);
	return ret;
}

void kset_release(struct kobject *kobj)
{
	struct kset *kset = container_of(kobj, struct kset, kobj);

	kfree(kset);
}

struct kset *kset_create(const char *name, struct kobject *parent_kobj)
{
	struct kset *kset;
	int retval;

	kset = kmalloc(sizeof(*kset));
	if (!kset)
		return NULL;

	kset->kobj.name = name;
	kset->kobj.parent = parent_kobj;
	kset->kobj.kset = NULL;

	return kset;
}

kset *kset_create_and_add(const char *name,
				 struct kobject *parent_kobj)
{
	struct kset *kset;
	int error;
	
	kset = kset_create(name, parent_kobj);
	if (!kset)
		return NULL;
	error = kset_register(kset);
	if (error) {
		kfree(kset);
		return NULL;
	}
	return kset;
}
