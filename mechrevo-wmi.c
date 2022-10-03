// SPDX-License-Identifier: GPL-2.0
/* WMI driver for Mechrevo Laptops */

//#include <linux/acpi.h>
//#include <linux/module.h>
#include <linux/wmi.h>
// It is said that you can change Performance Mode only when AC connected.
// https://tieba.baidu.com/p/7731917438
#include <linux/power_supply.h>

/*
Read:
cat /sys/devices/platform/PNP0C14:00/wmi_bus/wmi_bus-PNP0C14:00/ABBC0F6C-8EA1-11D1-00A0-C90629100000/performance_mode

Edit:
echo 1 | sudo tee /sys/devices/platform/PNP0C14:00/wmi_bus/wmi_bus-PNP0C14:00/ABBC0F6C-8EA1-11D1-00A0-C90629100000/performance_mode
*/
/* Fn+X 
 * 	45w Balance
 * 	35w Quiet 
 * 	54w Performace
 * 	*/

#define MECHREVO_WMI_GUID	"ABBC0F6C-8EA1-11d1-00A0-C90629100000"


/*
enum mechrevo_performance_mode {
	Balance = 0,
	Quiet = 1,
	Performance = 2, 
};
*/

static ssize_t performance_mode_store(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t count)
{
	int ret, status;
	unsigned long value;
	struct acpi_buffer in;
	struct power_supply * psy = power_supply_get_by_name("ACAD");
	if(NULL == psy){
		// if AC is not connected. How to notice user?
		return -EIO;
	}
	power_supply_put(psy);

	ret = kstrtoul(buf, 10 , &value);
	if(ret)
		return ret;
	if(value > 3 || value <0 )
		return -EINVAL;

	in.length = sizeof(unsigned long);
	in.pointer = &value;
	status = wmi_set_block(MECHREVO_WMI_GUID, 0x3, &in);

	return ACPI_SUCCESS(status) ?  count : -EIO;
}

static ssize_t performance_mode_show(struct device *dev, struct device_attribute *attr,
					char* buf)
{
	struct acpi_buffer out = {ACPI_ALLOCATE_BUFFER, NULL};
	union acpi_object *obj;
	acpi_status status;
	int ret;
	
	status = wmi_query_block(MECHREVO_WMI_GUID, 0x3 , &out);
	// struct wmi_device* wdev = dev_get_drvdata(dev);
	// union acpi_object *obj = wmidev_block_query(wdev,0x3);
	// obj->type obj->buffer.pointer
	if (ACPI_FAILURE(status))
		return -EIO;

	obj = (union acpi_object *) out.pointer;
	if (obj && obj->type == ACPI_TYPE_INTEGER)
		ret = sprintf(buf,"#Comment: 0->Balance 1->Quiet 2->Performance\n#Comment: editable only when AC connected\n%lld\n", obj->integer.value);
	else
		ret = -EIO;
	kfree(out.pointer);
	return ret;


}

static DEVICE_ATTR_RW(performance_mode);


static int mechrevo_wmi_probe(struct wmi_device *wdev, const void *context)
{
	if(!wmi_has_guid(MECHREVO_WMI_GUID)) {
		pr_warn("Mechrevo Management GUID not found\n");
		return -ENODEV;
	}
	return sysfs_create_file(&wdev->dev.kobj, &dev_attr_performance_mode.attr);
}

static void mechrevo_wmi_remove(struct wmi_device *wdev)
{
	return sysfs_remove_file(&wdev->dev.kobj, &dev_attr_performance_mode.attr);
}

// TODO: Notify?

static const struct wmi_device_id mechrevo_wmi_id_table[] = {
	{ MECHREVO_WMI_GUID , NULL } , 

	/* Terminating entry */
	{ }
};

static struct wmi_driver mechrevo_wmi_driver = {
	.driver = {
		.name = "mechrevo-wmi",
	},
	.id_table = mechrevo_wmi_id_table,
	.probe = mechrevo_wmi_probe,
	.remove = mechrevo_wmi_remove,
};
module_wmi_driver(mechrevo_wmi_driver);

//MODULE_DEVICE_TABLE(wmi, mechrevo_wmi_id_table);
MODULE_AUTHOR("jackyzy823");
MODULE_DESCRIPTION("Mechrevo WMI driver");
MODULE_LICENSE("GPL v2");
