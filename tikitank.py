#!/usr/bin/env python

import web
import json

from tank.Settings import Settings

render = web.template.render('templates')

s = Settings()

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
		return json.dumps(s.get_settings())

	def POST(self):
		s.post_settings(web.input())
		return self.GET()

class effects:
	def GET(self, kind):
		web.header('Content-Type', 'application/json')
		return json.dumps(s.get_effects(kind))

if __name__ == "__main__":
	app = web.application(urls, globals())
	app.run()

