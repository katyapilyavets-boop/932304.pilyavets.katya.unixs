#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/math64.h>

#define PROC_NAME "tsulab"

// Параметры падения
#define HEIGHT_M         300      // метров
#define G_100            981      // g = 9.81 м/с² → масштаб ×100

static struct proc_dir_entry *proc_file;

static ssize_t procfile_read(struct file *file, char __user *buffer,
                             size_t len, loff_t *offset)
{
    char msg[512];

    // Вычисляем t = sqrt(2h / g)
    // Для целочисленного sqrt: аргумент = (2 * h * 100 * 1_000_000) / g_100
    // → (2 * 300 * 100 * 1_000_000) / 981 = 60_000_000_000 / 981 = 61_162_079
    u64 arg = div64_u64(2ULL * HEIGHT_M * 100ULL * 1000000ULL, G_100);
    u32 t_ms = int_sqrt(arg);  // ≈ 7820 → 7.820 сек

    unsigned int t_sec = t_ms / 1000;
    unsigned int t_msec = t_ms % 1000;

    // Конечная скорость: v = g * t = 9.81 * t ≈ (981 * t_ms) / 100_000 м/с → в см/с:
    u32 v_cm_per_sec = (981ULL * t_ms) / 1000ULL;  // ≈ 7671 → 76.71 м/с

    // Энергия (m = 0.2 кг): E = 0.5 * m * v² = 0.1 * v² [Дж]
    // v = 76.71 м/с → v² = 5885 → E ≈ 588.5 Дж
    u32 energy_x10 = 5885;  // 588.5 Дж

    if (*offset > 0)
        return 0;

    int msg_len = snprintf(msg, sizeof(msg),
        "================================================\n"
        "  📱 TSU LAB: Phone Drop from Eiffel Tower\n"
        "================================================\n"
        "Initial height: %d meters\n"
        "Gravity: 9.81 m/s² (vacuum)\n\n"
        "Calculated fall time: %u.%03u seconds\n"
        "Impact velocity: %u.%02u m/s\n"
        "Kinetic energy (200 g phone): %u.%u J\n\n"
        "Note: In air, drag increases time to ~10–12 s.\n"
        "================================================\n",
        HEIGHT_M,
        t_sec, t_msec,
        v_cm_per_sec / 100, v_cm_per_sec % 100,
        energy_x10 / 10, energy_x10 % 10
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

static int __init tsu_module_init(void)
{
    proc_file = proc_create(PROC_NAME, 0444, NULL, &proc_fops);
    if (!proc_file)
        return -ENOMEM;

    pr_info("Welcome to the Tomsk State University\n");
    return 0;
}

static void __exit tsu_module_exit(void)
{
    proc_remove(proc_file);
    pr_info("Tomsk State University forever!\n");
}

module_init(tsu_module_init);
module_exit(tsu_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("TSU Physics & Astronomy");
MODULE_DESCRIPTION("How long does a phone fall from the Eiffel Tower?");