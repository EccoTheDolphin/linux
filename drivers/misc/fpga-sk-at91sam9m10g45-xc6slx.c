#include "fpga-sk-at91sam9m10g45-xc6slx.h"

struct sk_fpga fpga;

static int sk_fpga_open(struct inode *inode, struct file *file)
{
    if (!request_mem_region(fpga.fpga_mem_phys_start_cs0, fpga.fpga_cs_mem_window_size, "sk_fpga_cs0"))
    {
        printk(KERN_ERR"Failed to request mem region cs0\n");
        return -EBUSY;
    }
    fpga.fpga_mem_virt_start_cs0 = ioremap(fpga.fpga_mem_phys_start_cs0, 0xff);
    if (!fpga.fpga_mem_virt_start_cs0)
    {
        printk(KERN_ERR"Failed to ioremap mem region cs0\n");
        return -EBUSY;
    }
    return 0;
}

static int sk_fpga_close(struct inode *inodep, struct file *filp)
{
    if (fpga.fpga_mem_virt_start_cs0)
    {
        iounmap(fpga.fpga_mem_virt_start_cs0);
    }
    release_mem_region(fpga.fpga_mem_phys_start_cs0, fpga.fpga_cs_mem_window_size);
    return 0;
}

static ssize_t sk_fpga_write(struct file *file, const char __user *buf,
		             size_t len, loff_t *ppos)
{
    uint16_t data = 0;
    copy_from_user(&data, buf, sizeof(uint16_t));
    printk(KERN_ERR"Writing first short: %x\n", data);
    *((uint16_t*)fpga.fpga_mem_virt_start_cs0) = data;
    return len;
}

static ssize_t sk_fpga_read(struct file *file, char __user *buf,
		            size_t len, loff_t *ppos)
{
    printk(KERN_ERR"Reading first short: %x  %x\n", fpga.fpga_mem_virt_start_cs0, *((uint16_t*)fpga.fpga_mem_virt_start_cs0));
//    copy_to_user(buf, fpga.fpga_mem_virt_start_cs0, sizeof(uint16_t));
    return len; /* But we don't actually do anything with the data */
}

static const struct file_operations fpga_fops = {
        .owner                = THIS_MODULE,
        .write			= sk_fpga_write,
        .read			= sk_fpga_read,
        .open			= sk_fpga_open,
        .release		= sk_fpga_close,
};

static struct miscdevice sk_fpga_dev = {
        MISC_DYNAMIC_MINOR,
        "fpga",
        &fpga_fops
};


//TODO: remove magic numbers
int sk_fpga_setup_smc(void)
{
    uint32_t i = 0;
    int ret = -EIO;
    uint32_t* smc = NULL;
    uint32_t* ptr = NULL;
    uint32_t data = 0;
    if (!request_mem_region(0xFFFFE800, 0xff, "sk_fpga_smc0"))
    {
        printk(KERN_ERR"Failed to request mem region for smd\n");
        return ret;
    }
    smc = ioremap(0xFFFFE800, 0xff);
    if (!smc)
    {
        printk(KERN_ERR"Failed to ioremap mem region for smd\n");
        return ret;
    }
    ptr = smc;
    
    data = 0x01010101;
    *(ptr) = data;
    ptr++;
    data = 0x0a0a0a0a;
    *(ptr) = data;
    ptr++;
    data = 0x000e000e;
    *(ptr) = data;
    ptr++;
    data = 0x3 | 1 << 12;
    *(ptr) = data;
    ptr++;
 
    iounmap(smc);
    release_mem_region(0xFFFFE800, 0xff);
    
    return 0;    
}

int sk_fpga_fill_structure(struct platform_device *pdev)
{
    int ret = -EIO;
    // get fpga irq gpio
    fpga.fpga_irq_pin = of_get_named_gpio(pdev->dev.of_node, "fpga-irq-gpio", 0);
    if (!fpga.fpga_irq_pin) {
        dev_err(&pdev->dev, "Failed to obtain fpga irq pin\n");
        return ret;
    }

    // get fpga reset gpio
    fpga.fpga_reset_pin = of_get_named_gpio(pdev->dev.of_node, "fpga-reset-gpio", 0);
    if (!fpga.fpga_reset_pin) {
        dev_err(&pdev->dev, "Failed to obtain fpga reset pin\n");
        return ret;
    }

    // get fpga done gpio
    fpga.fpga_done = of_get_named_gpio(pdev->dev.of_node, "fpga-program-done", 0);
    if (!fpga.fpga_done) {
        dev_err(&pdev->dev, "Failed to obtain fpga done pin\n");
        return ret;
    }

    // get fpga cclk gpio
    fpga.fpga_cclk = of_get_named_gpio(pdev->dev.of_node, "fpga-program-cclk", 0);
    if (!fpga.fpga_cclk) {
        dev_err(&pdev->dev, "Failed to obtain fpga cclk pin\n");
        return ret;
    }

    // get fpga din gpio
    fpga.fpga_din = of_get_named_gpio(pdev->dev.of_node, "fpga-program-din", 0);
    if (!fpga.fpga_din) {
        dev_err(&pdev->dev, "Failed to obtain fpga din pin\n");
        return ret;
    }

    // get fpga prog gpio
    fpga.fpga_prog = of_get_named_gpio(pdev->dev.of_node, "fpga-program-prog", 0);
    if (!fpga.fpga_prog) {
        dev_err(&pdev->dev, "Failed to obtain fpga prog pin\n");
        return ret;
    }

    // get fpga mem sizes
    ret = of_property_read_u32(pdev->dev.of_node, "fpga-cs-memory-window-size", &fpga.fpga_cs_mem_window_size);
    if (ret != 0)
    {
        printk("Failed to obtain fpga cs memory window size from dtb\n");
        return -ENOMEM;
    }

    // get fpga start address for cs0
    ret = of_property_read_u32(pdev->dev.of_node, "fpga-cs0-memory-start-address", &fpga.fpga_mem_phys_start_cs0);
    if (ret != 0)
    {
        printk("Failed to obtain start phys mem start address for cs0 from dtb\n");
        return -ENOMEM;
    }

    // get fpga start address for cs1
    ret = of_property_read_u32(pdev->dev.of_node, "fpga-cs1-memory-start-address", &fpga.fpga_mem_phys_start_cs1);
    if (ret != 0)
    {
        printk("Failed to obtain start phys mem start address for cs1 from dtb\n");
        return -ENOMEM;
    }

    fpga.fpga_irq_num = -1;
    fpga.fpga_mem_virt_start_cs0 = NULL;
    fpga.fpga_mem_virt_start_cs1 = NULL;
    init_waitqueue_head(&fpga.fpga_wait_queue);

    // get fpga clk source
    fpga.fpga_clk = devm_clk_get(&pdev->dev, "mclk");
    if (IS_ERR(fpga.fpga_clk)) {
        dev_err(&pdev->dev, "Failed to get clk source for fpga from dtb\n");
        return ret;
    }
    return 0;
}

static int sk_fpga_probe (struct platform_device *pdev)
{
    int ret = -EIO;
    fpga.pdev = pdev;

    printk("Loading FPGA driver for SK-AT91SAM9M10G45EK-XC6SLX\n");

    // register misc device
    ret = misc_register(&sk_fpga_dev);
    if (ret)
    {
        printk(KERN_ERR"Unable to register \"fpga\" misc device\n");
        return -ENOMEM;
    }

    // fill structure by dtb info
    ret = sk_fpga_fill_structure(fpga.pdev);
    if (ret)
    {
        printk(KERN_ERR"Failed to fill fpga structure out of dts\n");
        return -EINVAL;
    }

    // run fpga clocking source
    ret = clk_set_rate(fpga.fpga_clk, 133333333);
    if (ret) 
    {
        dev_err(&pdev->dev, "Could not set fpga clk rate\n");
        return ret;
    }
    printk(KERN_ERR"Current clk rate: %ld\n", clk_get_rate(fpga.fpga_clk));
    ret = clk_prepare_enable(fpga.fpga_clk);
    if (ret)
    {
        dev_err(&pdev->dev, "Couldn't enable fpga clock\n");
    }

    // set reset ping to up   
    gpio_request(fpga.fpga_reset_pin, "sk_fpga_reset_pin");
    gpio_direction_output(fpga.fpga_reset_pin, 1);
    gpio_set_value(fpga.fpga_reset_pin, 1);

    ret = sk_fpga_setup_smc();
 
    return ret;
}

static int sk_fpga_remove(struct platform_device *pdev)
{
    printk(KERN_ALERT"Removing FPGA driver for SK-AT91SAM9M10G45EK-XC6SLX\n");
    // stop clocking source
    misc_deregister(&sk_fpga_dev);
    gpio_free(fpga.fpga_reset_pin);
    return 0;
}

static const struct of_device_id sk_fpga_of_match_table[] = {
    { .compatible = "sk,at91-xc6slx", },
    { /* end of list */ }
};
MODULE_DEVICE_TABLE(of, sk_fpga_of_match_table);

static struct platform_driver sk_fpga_driver = {
    .probe         = sk_fpga_probe,
    .remove        = sk_fpga_remove,
    .driver        = {
        .owner          = THIS_MODULE,
        .name           = "fpga",
        .of_match_table = of_match_ptr(sk_fpga_of_match_table),
    },
};

module_platform_driver(sk_fpga_driver);
MODULE_AUTHOR("Alexey Baturo <baturo.alexey@gmail.com>");
MODULE_DESCRIPTION("Driver for Xilinx Spartan6 xc6slx16 fpga for StarterKit AT91SAM9M10G45EK-XC6SLX board");
MODULE_LICENSE("GPL v2");
