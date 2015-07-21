#!/usr/bin/env python

from Adafruit_I2C import Adafruit_I2C
from Adafruit_BBIO.SPI import SPI
from tank import Rainbow
import time

class PwmLed:
	# Registers
	__MODE1         = 0x00
	__MODE2         = 0x01
	__SUBADR1       = 0x02
	__SUBADR2       = 0x03
	__SUBADR3       = 0x04
	__ALLCALLADR    = 0x05
	__LED0_ON_L     = 0x06
	__LED0_ON_H     = 0x07
	__LED0_OFF_L    = 0x08
	__LED0_OFF_H    = 0x09
	__ALL_LED_ON_L  = 0xFA
	__ALL_LED_ON_H  = 0xFB
	__ALL_LED_OFF_L = 0xFC
	__ALL_LED_OFF_H = 0xFD
	__PRESCALE      = 0xFE

	# Mode 1 Bits
	__RESTART       = 0x80
	__SLEEP         = 0x10
	__ALLCALL       = 0x01

	# Mode 2 Bits
	__INVRT         = 0x10
	__OUTDRV        = 0x04

	def __init__(self, address):
		self.i2c = Adafruit_I2C(address)
		self.brightness = 8

		# Ensure board is connected
		if -1 == self.i2c.readU8(self.__MODE1):
			raise IOError("Error opening PCA9685, Addr: 0x%02X" % (address))

		# Ensure everything is off
		self.close()

		# Configure for use w/external LED driver
		self.i2c.write8(self.__MODE2, self.__OUTDRV)

		# Enable oscillator
		mode1 = self.i2c.readU8(self.__MODE1)
		mode1 = mode1 & ~self.__SLEEP
		self.i2c.write8(self.__MODE1, mode1)
		time.sleep(0.001)

		# Query the pwm frequency
		prescale = self.i2c.readU8(self.__PRESCALE)
		freq = 25000000 / (4096 * (prescale + 1))

		print 'Opened PCA9685, Addr: 0x%0.2X, Freq: %sHz' % (address, freq)

	def close(self):
		# Turn off all pwm
		self.i2c.write8(self.__ALL_LED_ON_L, 1)
		self.i2c.write8(self.__ALL_LED_ON_H, 0)
		self.i2c.write8(self.__ALL_LED_OFF_L, 1)
		self.i2c.write8(self.__ALL_LED_OFF_H, 0)

		# Disable oscillator
		self.i2c.write8(self.__MODE1, self.__SLEEP | self.__ALLCALL)
		time.sleep(0.001)

	def render(self, pixels):

		# On time always stays at 1
		# Off time of 0 means always on
		# Off time of 1 means always off
		# Scale our 0-255 by brightness and
		# Just add 1 to the [0-4095] value we get

		for i,v in enumerate(pixels):
			v = (v * 2 * self.brightness + 1) & 0xFFF
			n = i * 4

			self.i2c.write8(self.__LED0_OFF_L + n, v & 0xff)
			self.i2c.write8(self.__LED0_OFF_H + n, v >> 8)

class SpiLed:
	__SPEED  = 16000000
	__MODE   = 0
        __DEVICE = 0

	def __init__(self, bus):
		# Use Adafruit_BBIO.SPI to initialize the cap
		# and the spi bus configuration
		s = SPI(bus, self.__DEVICE)
		s.msh = self.__SPEED
		s.mode = self.__MODE
		s.close()

		# Use normal file for writing bytes
		dev = '/dev/spidev%s.%s' % (bus + 1, self.__DEVICE)
		self.spi = open(dev, 'wb')
		print 'Opened %s, Freq: %sHz' % (dev, self.__SPEED)

	def render(self, pixels):
		self.spi.write(pixels)
		self.spi.flush()


	def close(self):
		self.spi.close()

if __name__ == "__main__":
	s1 = SpiLed(0)
	s2 = SpiLed(1)
	#p1 = PwmLed(0x40)
	#p2 = PwmLed(0x41)

	colors = Rainbow.fastled_rainbow

	cc = Rainbow.TypicalSMD5050

	i = 0
	cnt = len(colors)

	buf = bytearray((3*cnt) + 15)
	for x in range(0, (3*cnt)):
		buf[x] = 0x80


	s2.render(buf)

	try:
		while True:
			#r,g,b = Rainbow.color_correct(colors[i], cc)

			for x in range(0, cnt):
				r,g,b = colors[(i + x) % cnt]
				buf[3*x+0] = (g >> 1) | 0x80
				buf[3*x+1] = (r >> 1) | 0x80
				buf[3*x+2] = (b >> 1) | 0x80

			s1.render(buf)
			s2.render(buf)
			#p1.render([ r, g, b, r, g, b, r, g, b, r, g, b, r, g, b ])
			#p2.render([ r, g, b, r, g, b, r, g, b, r, g, b, r, g, b ])
			i = (i + 1) % len(colors)
			#time.sleep(0.050)
			#time.sleep(0.01)
	except KeyboardInterrupt:
		s1.close()
		s2.close()
		#p1.close()
		#p2.close()

