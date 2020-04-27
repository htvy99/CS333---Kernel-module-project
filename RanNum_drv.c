#include <linux/module.h>	//dinh nghia cac macro nhu module_init
#include <linux/fs.h>		//dinh nghia ham cap phat/giai phong device number
#include <linux/device.h>	//chua cac ham phuc vu viec tao device file
#include <linux/slab.h>		//chua cac ham kmalloc & kfree de cap phat bo nho

#include "RanNum_drv.h"	//thu vien mo ta cac thanh ghi cua RandomNumber device

#define DRIVER_AUTHOR "Vy Huynh <1751027> | Trinh Ngo <nlptrinh@apcs.vn> | Truc Nguyen <1751112>"
#define DRIVER_DESC "Generate random number module"

typedef struct vchar_dev {
	unsigned char * control_regs;
	unsigned char * status_regs;
	unsigned char * data_regs;
} vchar_dev_t;

struct _vchar_drv
{
	dev_t dev_num;
	struct class *dev_class;
	struct device *dev;
	vchar_dev_t * vchar_hw;
} vchar_drv;

//**************** device specific - START ***************/
// Ham khoi tao thiet bi
int vchar_hw_init(vchar_dev_t *hw)
{
	char * buf;
	buf = kzalloc(NUM_DEV_REGS * REG_SIZE, GFP_KERNEL);
	if(!buf) {
		return -ENOMEM;
	}

	hw->control_regs = buf;
	hw->status_regs = hw->control_regs + NUM_CTRL_REGS;
	hw->data_regs = hw->status_regs + NUM_STS_REGS;

	//khoi tao gia tri ban dau cho cac thanh ghi
	hw->control_regs[CONTROL_ACCESS_REG] = 0x03;
	hw->status_regs[DEVICE_STATUS_REG] = 0x03;

	return 0;
}

// ham giai phong thiet bi
void vchar_hw_exit(vchar_dev_t *hw)
{
	kfree(hw->control_regs);
}

// ham doc tu cac thanh ghi du lieu cua thiet bi

// ham doc tu thanh ghi trang thai cua thiet bi

// ham ghi vao cac thanh ghi dieu khien cua thiet bi

/************************* device specific - END **************/

/********************* OS specific - START ********************/
// cac ham entry point

// ham khoi tao driver
static int __init RanNum_driver_init(void)
{
	int ret = 0;
	
	// cap phat device number
	vchar_drv.dev_num = 0;
	ret = alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, "RanNum_device");
	if (ret < 0) {
		printk("Failed to register device number\n");
		goto failed_register_devnum;
	}
	printk("Allocated device number (%d,%d)\n", MAJOR(vchar_drv.dev_num), MINOR(vchar_drv.dev_num));

	// tao device file
	vchar_drv.dev_class = class_create(THIS_MODULE, "class_RanNum_dev");
	if(vchar_drv.dev_class == NULL) {
		printk("Failed to create a device class\n");
		goto failed_create_class;	
	}
	
	vchar_drv.dev = device_create(vchar_drv.dev_class, NULL, vchar_drv.dev_num, NULL, "RanNum_dev");
	if(IS_ERR(vchar_drv.dev)) {
		printk("Failed to create a device\n");
		goto failed_create_device;
	}

	// cap phat bo nho cho cau truc du lieu cua character driver
	vchar_drv.vchar_hw = kzalloc(sizeof(vchar_dev_t), GFP_KERNEL);
	if(!vchar_drv.vchar_hw) {
		printk("Failed to allocate data structure of driver\n");
		ret = -ENOMEM;
		goto failed_allocate_structure;
	}

	// khoi tao thiet bi vat li
	ret = vchar_hw_init(vchar_drv.vchar_hw);
	if (ret < 0) {
		printk("Failed to initialize a virtual character device\n");
		goto failed_init_hw;
	}

	// dki entry points voi kernel

	// dki ham xu ly ngat

	printk("Initialize RanNum driver successfully\n");
	return 0;

failed_init_hw:
	kfree(vchar_drv.vchar_hw);
failed_allocate_structure:
	device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
failed_create_device:
	class_destroy(vchar_drv.dev_class);
failed_create_class:
	unregister_chrdev_region(vchar_drv.dev_num, 1);
failed_register_devnum:
	return ret;
}

// ham ket thuc driver
static void __exit RanNum_driver_exit(void)
{
	//huy dang ky xu li ngat
	
	//huy dang ky entry point voi kerel

	//giai phong thiet bi vat ly
	vchar_hw_exit(vchar_drv.vchar_hw);

	//giai phong bo nho da cap phat du lieu cua driver
	kfree(vchar_drv.vchar_hw);

	//xoa bo device file
	device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
	class_destroy(vchar_drv.dev_class);


	//giai phong device number
	unregister_chrdev_region(vchar_drv.dev_num, 1);

	printk("Exit RanNum driver\n");
}
/*************************OS specific - END *************************/


module_init(RanNum_driver_init);
module_exit(RanNum_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("testdevice");
