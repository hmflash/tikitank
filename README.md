### Yet another implementation of the tiki tank software.

### Setup Notes

* Flash Image ```bone-debian-7.8-lxde-4gb-armhf-2015-03-01-4gb.img```

* Disable HDMI in ```/boot/uEnv.txt```
```
#Disable HDMI
cape_disable=capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN
```

* Disable lightdm in ```/boot/uEnv.txt```
```
# For BBB Debian OS
# This disables lightdm run from "/etc/init.d/lightdm".
optargs=text
```

### Wiring Diagram

Beaglebone:
```
P9_18 SPI0_DATA
P9_22 SPI0_CLOCK
P9_30 SPI1_DATA
P9_31 SPI_CLOCK
P9_19 I2C2_SCL
P9_20 I2C2_SDA
```

49E Hall Sensor
```
P9_3  VDD_3V3
P9_1  DGND
P9_39 AIN0 (3.3k x2 voltage divider)
```

LPD8806 Connector

| 5V | CLK | DATA | GND |
|----|-----|------|-----|

I2C Connector

| 3.3V | SDA | SCL | GND |
|------|-----|-----|-----|

Hall Connector

| OUT | GNG | 3.3V |
|-----|-----|------|

### Useful Links

1. [Beaglebone PRU connections and modes](http://elinux.org/Ti_AM33XX_PRUSSv2#PRU_to_external_peripherals)
1. [Overlay Pin Modes](https://github.com/cdsteinkuehler/beaglebone-universal-io/blob/master/cape-universal-00A0.dts)
1. [System Reference Manual](http://www.adafruit.com/datasheets/BBB_SRM.pdf)
1. [Cape Expansion Headers](http://elinux.org/Beagleboard:Cape_Expansion_Headers)
1. [AM335x PRU Package](https://github.com/rjw245/am335x_pru_package)
1. [PRU Assembly Instructions](http://processors.wiki.ti.com/index.php/PRU_Assembly_Instructions)
1. [Programmable Realitme Unit](http://processors.wiki.ti.com/index.php/Programmable_Realtime_Unit)
1. [PRU PRojects](http://processors.wiki.ti.com/index.php/PRU_Projects)
1. [Cape Firmwares](https://github.com/jadonk/cape-firmware)
