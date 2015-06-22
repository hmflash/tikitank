#!/usr/bin/env python

import web
import json

from tank import Effect
from tank.effects import *

opts = {
	'dmxBrightness' : 100,
	'manualTick' : 0,
	'idleInterval' : 120,
}

render = web.template.render('templates')

urls = (
	'/', 'index',
	'/settings', 'settings',
	'/api/(\w+)/effects', 'effects',
)

class index:
	def GET(self):
		return render.index()

class settings:
	def GET(self):
		web.header('Content-Type', 'application/json')
		return json.dumps(opts)

	def POST(self):
		data = web.input()
		for k in opts.keys():
			opts[k] = getattr(data, k, opts[k])
		return self.GET()

class effects:
	def GET(self, kind):
		web.header('Content-Type', 'application/json')
		ret = []
		for k,v in Effect.effects.iteritems():
			if kind != v.kind:
				continue

			ret.append({
				'name' : v.__doc__,
				'id' : k,
				'isSensorDriven' : v.isSensorDriven,
			})
		return json.dumps(ret)

if __name__ == "__main__":
	app = web.application(urls, globals())
	app.run()

