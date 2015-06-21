#!/usr/bin/env python

import web
import json

opts = {
	'dmxBrightness' : 100,
	'manualTick' : 0,
	'idleInterval' : 120,
}

render = web.template.render('templates')

urls = (
	'/', 'index',
	'/settings', 'settings',
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

if __name__ == "__main__":
	app = web.application(urls, globals())
	app.run()

