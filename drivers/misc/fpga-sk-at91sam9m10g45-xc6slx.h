#ifndef SK_FPGA_DRIVER_HEADER
#define SK_FPGA_DRIVER_HEADER

// TODO: I don't think we need so many includes

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <generated/utsrelease.h>
#include <linux/miscdevice.h>
#include <linux/spinlock.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/wait.h>

#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/ioctl.h>
#include <linux/platform_data/atmel.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/suspend.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/of_gpio.h>


#define SMC 0xFFFFE800
#define SMC_SETUP(num) (0x10 * num + 0x00)
#define SMC_PULSE(num) (0x10 * num + 0x04)
#define SMC_CYCLE(num) (0x10 * num + 0x08)
#define SMC_MODE(num)  (0x10 * num + 0x0C)
#define SMC_DELAY1 0xC0
#define SMC_DELAY2 0xC4
#define SMC_DELAY3 0xC8
#define SMC_DELAY4 0xCC
#define SMC_DELAY5 0xD0
#define SMC_DELAY6 0xD4
#define SMC_DELAY7 0xD8
#define SMC_DELAY8 0xDC

struct sk_fpga
{
    struct platform_device *pdev;
    uint32_t fpga_cs_mem_window_size; // phys mem size on any cs pin
    uint32_t fpga_mem_phys_start_cs0; // phys mapped addr of fpga mem on cs0
    uint32_t fpga_mem_phys_start_cs1; // phys mapped addr of fpga mem on cs1
    uint8_t *fpga_mem_virt_start_cs0; // virt mapped addr of fpga mem on cs0
    uint8_t *fpga_mem_virt_start_cs1; // virt mapped addr of fpga mem on cs0
    uint8_t fpga_irq_pin;             // pin to recieve irq from fpga on arm
    uint8_t fpga_reset_pin;           // pin to reset fpga internal state
    uint8_t fpga_cclk;                // pin to run cclk on fpga
    uint8_t fpga_din;                 // pin to set data to fpga
    uint8_t fpga_done;                // pin to read status done from fpga
    uint8_t fpga_prog;                // pin to set mode to prog on fpga
    wait_queue_head_t fpga_wait_queue;
    int32_t fpga_irq_num;             // where to store assigned interrupt
    struct clk* fpga_clk;             // clock source for fpga
};

static int sk_fpga_remove (struct platform_device *pdev);
static int sk_fpga_probe  (struct platform_device *pdev);

// setup clk for fpga
int sk_fpga_start_clk(struct platform_device *pdev);

#endif
