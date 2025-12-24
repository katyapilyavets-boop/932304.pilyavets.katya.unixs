#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/math64.h>

#define PROC_NAME "tsulab"

// Физические константы
#define GRAVITY 9.81               // м/с²
#define EIFFEL_TOWER_HEIGHT 300.0  // метров (до верхней смотровой площадки)

// Упрощённый расчёт времени падения: t = sqrt(2h / g)
// И конечной скорости: v = g * t

static struct proc_dir_entry *proc_file;


#define FALL_TIME_SECONDS      7.82   // sqrt(2 * 300 / 9.81) ≈ 7.82 с
#define FINAL_SPEED_M_PER_SEC  76.7   // 9.81 * 7.82 ≈ 76.7 м/с (~276 км/ч)

static ssize_t procfile_read(struct file *file, char __user *buffer,
                             size_t len, loff_t *offset)
{
    char msg[512];

    if (*offset > 0)
        return 0;

    int msg_len = snprintf(msg, sizeof(msg),
        "========================== PHONE FALL SIMULATOR ==========================\n"
        "                 Tomsk State University - Physics Lab\n\n"
        "Scenario: Smartphone dropped from the top of the Eiffel Tower (300 m)\n"
        "Assumptions: Vacuum (no air resistance), g = 9.81 m/s²\n\n"
        "Time of fall:       %.2f seconds\n"
        "Impact speed:       %.1f m/s (%.0f km/h)\n"
        "Distance fallen:    %.0f meters\n\n"
        "Note: In reality, air resistance would limit terminal velocity to ~50 m/s.\n"
        "Your phone would be destroyed either way. Do not try this in Paris.\n"
        "=========================================================================\n",
        FALL_TIME_SECONDS,
        FINAL_SPEED_M_PER_SEC,
        FINAL_SPEED_M_PER_SEC * 3.6,
        EIFFEL_TOWER_HEIGHT
    );

    if (copy_to_user(buffer, msg, msg_len))
        return -EFAULT;

    *offset = msg_len;
    return msg_len;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_fops = {
    .proc_read = procfile_read,
};
#else
static const struct file_operations proc_fops = {
    .read = procfile_read,
};
#endif

static int __init phonefall_init(void)
{
    proc_file = proc_create(PROC_NAME, 0444, NULL, &proc_fops);
    if (!proc_file)
        return -ENOMEM;

    pr_info("TSU Physics Lab: PhoneFall module loaded. Don't drop your phone!\n");
    return 0;
}

static void __exit phonefall_exit(void)
{
    proc_remove(proc_file);
    pr_info("PhoneFall module unloaded. Your phone is safe (for now).\n");
}

module_init(phonefall_init);
module_exit(phonefall_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TSU Physics Department");
MODULE_DESCRIPTION("Simulates phone falling from the Eiffel Tower");