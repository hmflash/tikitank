import time, select
from select import poll

import ctypes, os, gc

CLOCK_MONOTONIC_RAW = 4 # see <linux/time.h>

class timespec(ctypes.Structure):
    _fields_ = [
        ('tv_sec', ctypes.c_long),
        ('tv_nsec', ctypes.c_long)
    ]

librt = ctypes.CDLL('librt.so.1', use_errno=True)
clock_gettime = librt.clock_gettime
clock_gettime.argtypes = [ctypes.c_int, ctypes.POINTER(timespec)]

def monotonic_time():
    t = timespec()
    if clock_gettime(CLOCK_MONOTONIC_RAW, ctypes.pointer(t)) != 0:
        errno_ = ctypes.get_errno()
        raise OSError(errno_, os.strerror(errno_))
    return (t.tv_sec * 1000) + (t.tv_nsec / 1000000)

import Rainbow

# Abount 66 FPS
FRAME_DELAY = 25

class Render:
	def __init__(self, pipe):
		self._pipe = pipe
		self._next = 0
		self._total_dropped = 0
		self._frame_count = 0
		self._treads_len = 32 * 15
		self._treads = bytearray(3 * self._treads_len + 15)
		self._r, self._w = os.pipe()
		self._poll = poll()
		self._poll.register(self._r, select.POLLIN)
		gc.disable()

	def __enter__(self):
		return self

	def __exit__(self, type, value, tb):
		self.close()

	def run(self):
		#self._next = time.time() + FRAME_DELAY
		self._next = monotonic_time() + FRAME_DELAY

		while True:
			self._update_speed()

			self._compute_frame()

			self._wait_for_vsync()

			#self._draw_frame()

			self._handle_ipc()

	def close(self):
		self._pipe.close()
		print 'Renderer shut down gracefully'

	def _wait_for_vsync(self):
		now = monotonic_time()
		timeout = self._next - now

		if timeout < 0:
			num_dropped = ((now - self._next) / FRAME_DELAY) + 1
			print 'Effect is too slow, dropping %s frames (now=%s, next=%s)' % (num_dropped, now, self._next)
			self._next += FRAME_DELAY * num_dropped
			self._total_dropped += num_dropped
			self._frame_count += num_dropped
			timeout = self._next - now

		while monotonic_time() < self._next:
			pass
		#self._poll.poll(timeout)

		self._next += FRAME_DELAY
		self._frame_count += 1

	def _update_speed(self):
		pass

	def _compute_frame(self):
		start = monotonic_time()

		mcnt = len(Rainbow.rainbow)
		for i in xrange(self._treads_len):
			idx = i % mcnt
			r,g,b = Rainbow.rainbow[idx]
			idx = 3 * i
			self._treads[idx + 0] = r / 2 + 1
			self._treads[idx + 1] = g / 2 + 1
			self._treads[idx + 2] = b / 2 + 1

		end = monotonic_time() - start
		if end > 20:
			print 'Render out: %d' % end

	def _draw_frame(self):
		pass

	def _handle_ipc(self):
		if self._pipe.poll():
			o = self._pipe.recv()
			print 'Got command: %s' % o

def main(pipe):
	try:
		with Render(pipe) as r:
			r.run()
	except KeyboardInterrupt:
		pass
