#define pr_fmt(fmt) KBUILD_MODNAME ":" fmt

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/param.h>

#define HELLO_LEN 13

static char my_str[HELLO_LEN + 1] = {0};
static int idx;
static int ch_val;

static int set_idx(const char *val, const struct kernel_param *kp) {
	int ret;
	int new_idx;

	ret = kstrtoint(val, 10, &new_idx);
	if (ret) {
		return ret;
	}

	if (new_idx < 0 || new_idx >= HELLO_LEN) {
		return -EINVAL;
	}

	idx = new_idx;

	return 0;
}

static int get_idx(char *buffer, const struct kernel_param *kp) {
	return param_get_int(buffer, kp);
}

static const struct kernel_param_ops idx_ops = {
	.set = set_idx,
	.get = get_idx,
};

static int set_ch_val(const char *val, const struct kernel_param *kp) {
	int ret;
	int new_ch_val;

	ret = kstrtoint(val, 10, &new_ch_val);
	if (ret) {
		return ret;
	}

	if (new_ch_val < 32 || new_ch_val > 126) {
		return -EINVAL;
	}

	ch_val = new_ch_val;
	my_str[idx] = (char)ch_val;

	return 0;
}

static int get_ch_val(char *buffer, const struct kernel_param *kp) {
	return param_get_int(buffer, kp);
}

static const struct kernel_param_ops ch_val_ops = {
	.set = set_ch_val,
	.get = get_ch_val,
};

static int __init hello_world_init(void) {
	pr_info("init\n");

	return 0;
}

static void __exit hello_world_exit(void) {
	pr_info("exit\n");
}

module_param_cb(idx, &idx_ops, &idx, 0644);
MODULE_PARM_DESC(idx, "Index of character in my_str");

module_param_cb(ch_val, &ch_val_ops, &ch_val, 0644);
MODULE_PARM_DESC(ch_val, "Visible ASCII code of character");

module_param_string(my_str, my_str, sizeof(my_str), 0444);
MODULE_PARM_DESC(my_str, "String written character by character");

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL 3.0");
MODULE_AUTHOR("c0un7z3r0");
MODULE_DESCRIPTION("Parameterized Hello World kernel module");
