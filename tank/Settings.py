import json

from tank import Effect
from tank.effects import *

class Settings:
	settings_file = 'settings.json'

	def __init__(self):
		self._effects = {}
		self._settings = {}

		for k,v in Effect.effects.iteritems():
			items = self._effects.get(v.kind, [])
			items.append({
				'name' : v.__doc__,
				'id' : k,
				'isSensorDriven' : v.isSensorDriven,
			})
			self._effects[v.kind] = items

		self._load_settings()

	def _load_settings(self):
		try:
			with open(Settings.settings_file, 'r') as f:
				saved = json.load(f)
		except:
			saved = {}

		self.post_settings(saved.get('settings', {}))

	def _save_settings(self):
		obj = {
			'settings' : self._settings,
		}

		try:
			with open(Settings.settings_file, 'w') as f:
				json.dump(obj, f, indent = 2)
		except:
			pass
		
	def _apply_setting(self, name, data, max_val, def_val):
		cur = self._settings.get(name, None)
		new = data.get(name, None)

		try:
			new = int(new) if new else None
		except:
			val = None

		if new >= 0 and new <= max_val:
			self._settings[name] = new;
			if cur and cur != new:
				self._save_settings()
		elif not cur:
			self._settings[name] = def_val


	def post_settings(self, data):
		self._apply_setting('dmxBrightness', data, 100,   100)
		self._apply_setting('manualTick',    data, 65536, 0)
		self._apply_setting('idleInterval',  data, 65535, 120)

	def get_settings(self):
		return self._settings

	def get_effects(self, kind):
		return self._effects.get(kind, [])
