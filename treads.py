#!/usr/bin/env python

import beaglebone_pru_adc as adc
import subprocess, os.path
import sys, time

class SpeedSensor(adc.Capture):
	def __init__(self, *k, **kw):
		super(SpeedSensor, self).__init__(*k, **kw)

	def start(self, firmware_src):
		fw_bin = os.path.splitext(firmware_src)[0]
		cmd = [ 'pasm', '-V3', '-b', firmware_src, fw_bin ]
		p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
		(stdout, stderr) = p.communicate()

		if p.returncode != 0:
			print stdout
			raise IOError('Failed to compile "%s"' % firmware_src)

		fw = os.path.abspath(fw_bin + '.bin')
		adc._pru_adc.Capture.start(self, fw)

	def close(self):
		self.stop()
		self.wait()
		super(SpeedSensor, self).close()

if __name__ == '__main__':
	s = SpeedSensor()

	if len(sys.argv) > 1:
		s.encoder0_threshold = 4096
		s.start('tank/firmware.p')

		time.sleep(10)
		timer = s.timer
		_, min0, max0, _, _ = s.encoder0_values
		s.close()

		print 'Capture runs at %d readings per second' % (timer // 10)
		print 'Time value of one timer unit is %d nanosec' % (1000000000 // timer)
		print 'Range for the encoder:', min0, '-', max0
		print 'Recommended threshold value for encoder is:', int(0.9*(max0-min0))
		sys.exit()
	else:
		s.encoder0_threshold = 750
		s.encoder0_delay = 100
		s.start('tank/firmware.p')

	num = 0
	last = s.encoder0_ticks

	try:
		while num < 10:
			#print s.timer, s.encoder0_values
			#continue

			val = s.encoder0_ticks

			if last != val:
				delta = val - last
				num += delta
				print 'Tick: %s' % delta
				last = val

	except KeyboardInterrupt:
		s.close()

