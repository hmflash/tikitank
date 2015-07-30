#!/usr/bin/env python

from multiprocessing import Process, Pipe

import web
import json, os, select, time
from threading import Lock
from tank.Settings import Settings
from tank import Render

render = web.template.render('templates')

urls = (
	'/', 'index',
	'/settings', 'settings',
	'/api/(\w+)/effects', 'effects',
	'/api/(\w+)/effect', 'effect',
)

class index:
	def GET(self):
		return render.index()

class settings:
	def GET(self):
		with web.config.lock:
			s = web.config.settings
			web.header('Content-Type', 'application/json')
			return json.dumps(s.get_settings())

	def POST(self):
		with web.config.lock:
			s = web.config.settings
			s.post_settings(web.input())
			web.config.pipe.send(['settings', s.get_settings()])
		return self.GET()

class effects:
	def GET(self, kind):
		with web.config.lock:
			s = web.config.settings
			web.header('Content-Type', 'application/json')
			return json.dumps(s.get_effects(kind))

class effect:
	def GET(self, kind):
		with web.config.lock:
			s = web.config.settings
			web.header('Content-Type', 'application/json')
			return json.dumps(s.get_effect(kind))

	def POST(self, kind):
		with web.config.lock:
			s = web.config.settings
			s.set_effect(kind, web.input())
			web.config.pipe.send([str(kind), s.get_effect(kind)])
		return self.GET(kind)

if __name__ == "__main__":
	parent, child = Pipe()

	worker = Process(target=Render.main, name='Render', args=(child,))
	worker.start()

	child.close()

	web.config.pipe = parent
	web.config.settings = Settings()
	web.config.lock = Lock()
	web.config.debug = False

	app = web.application(urls, globals())
	app.run()

	worker.join()
	parent.close()
