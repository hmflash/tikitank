#!/usr/bin/env python

import beaglebone_pru_adc as adc
import subprocess, os.path
import sys, time

class SpeedSensor(adc.Capture):
	def __init__(self, firmware, *k, **kw):
		super(SpeedSensor, self).__init__(*k, **kw)

		fw = os.path.splitext(firmware)[0]
		cmd = [ 'pasm', '-V3', '-b', firmware, fw ]
		p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
		(stdout, stderr) = p.communicate()

		if p.returncode != 0:
			print stdout
			raise IOError('Failed to compile "%s"' % firmware)

		self._firmware = fw = os.path.abspath(fw + '.bin')

		self.encoder0_pin = kw.get('pin', 0)

	def calibrate(self):
		print 'Running encoder for 10 seconds...'

		self.encoder0_threshold = 4096
		self.encoder0_delay = 0
		adc._pru_adc.Capture.start(self, self._firmware)

		time.sleep(10)
		timer = s.timer
		_, min0, max0, _, _ = s.encoder0_values

		self.close()

		print 'Capture runs at %d readings per second' % (timer // 10)
		print 'Time value of one timer unit is %d nanosec' % (1000000000 // timer)
		print 'Range for the encoder:', min0, '-', max0
		print 'Recommended threshold value for encoder is:', int(0.9*(max0-min0))

	def start(self):
		s.encoder0_threshold = 750
		s.encoder0_delay = 100
		adc._pru_adc.Capture.start(self, self._firmware)

	def close(self):
		self.stop()
		self.wait()
		super(SpeedSensor, self).close()

if __name__ == '__main__':
	s = SpeedSensor('tank/firmware.p')

	if '--test' in sys.argv:
		s.calibrate()
		sys.exit()

	try:
		s.start()

		if '-v' in sys.argv:
			while True:
				print s.timer, s.encoder0_values

		num = 0

		while True:
			delta = s.encoder0_ticks - num
			if delta:
				print 'Tick: %s (%s)' % (delta, num)
				num += delta

	except KeyboardInterrupt:
		s.close()

