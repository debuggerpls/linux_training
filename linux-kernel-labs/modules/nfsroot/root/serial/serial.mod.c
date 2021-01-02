#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0xcfcf5421, "module_layout" },
	{ 0x19697d3d, "platform_driver_unregister" },
	{ 0x589d6198, "__platform_driver_register" },
	{ 0xc815ecc5, "_dev_err" },
	{ 0x38aeefda, "debugfs_create_u32" },
	{ 0xbe6830bb, "debugfs_create_dir" },
	{ 0x18ae20ca, "misc_register" },
	{ 0xc02231ec, "devm_kasprintf" },
	{ 0x5bbe49f4, "__init_waitqueue_head" },
	{ 0xe426c208, "devm_request_threaded_irq" },
	{ 0x29828429, "platform_get_irq" },
	{ 0xdc43b387, "of_property_read_variable_u32_array" },
	{ 0x358e6b82, "devm_ioremap_resource" },
	{ 0x458c3ef7, "devm_kmalloc" },
	{ 0x49ef588e, "platform_get_resource" },
	{ 0x74074de5, "__pm_runtime_resume" },
	{ 0x7c9f3f77, "pm_runtime_enable" },
	{ 0x28118cb6, "__get_user_1" },
	{ 0x822137e2, "arm_heavy_mb" },
	{ 0xa9b1fef9, "__dynamic_dev_dbg" },
	{ 0x3dcf1ffa, "__wake_up" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x86332725, "__stack_chk_fail" },
	{ 0x49970de8, "finish_wait" },
	{ 0x647af474, "prepare_to_wait_event" },
	{ 0x1000e51, "schedule" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0xbb72d4fe, "__put_user_1" },
	{ 0x89f42aa7, "__pm_runtime_disable" },
	{ 0x570426ad, "misc_deregister" },
	{ 0xeb7e14b4, "debugfs_remove" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x39a12ca7, "_raw_spin_unlock_irqrestore" },
	{ 0xbc10dd97, "__put_user_4" },
	{ 0x5f849a69, "_raw_spin_lock_irqsave" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cbootlin,serial");
MODULE_ALIAS("of:N*T*Cbootlin,serialC*");

MODULE_INFO(srcversion, "3295963428FCF97EDFD7E4D");
