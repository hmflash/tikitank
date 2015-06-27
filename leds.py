#!/usr/bin/env python

from Adafruit_I2C import Adafruit_I2C
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

def white_shades():
	return [ (c, c, c) for x in range(0,256,1) + range(255,-1,-1) ]

def simple_colorwheel():
	ret = []

	r = 255
	g = 0
	b = 0
	dr = -1
	dg = 1
	db = 0

	while True:
		ret.append((r,g,b))

		r += dr
		g += dg
		b += db

		if g == 256:
			r = 0
			g = 255
			b = 0
			dr = 0
			dg = -1
			db = 1

		if b == 256:
			r = 0
			g = 0
			b = 255
			dr = 1
			dg = 0
			db = -1

		if r == 256:
			return ret;

if __name__ == "__main__":
	p = PwmLed(0x40)

	colors = Rainbow.fastled_rainbow()

	cc = Rainbow.TypicalSMD5050

	i = 0
	try:
		while True:
			r,g,b = Rainbow.color_correct(colors[i], cc)
			p.render([ r, g, b ])
			i = (i + 1) % len(colors)
			time.sleep(0.050)
	except KeyboardInterrupt:
		p.close()
