import json
from collections import OrderedDict

from tank import Effect
from tank.effects import *

class Settings:
	settings_file = 'settings.json'
	kinds = [ Effect.BARREL, Effect.TREADS, Effect.PANELS ]

	def __init__(self):
		self._effects = {}
		self._settings = {}
		self._active = {}

		# Load implemented effects
		for k,v in Effect.effects.iteritems():
			if v.kind not in Settings.kinds:
				continue

			items = self._effects.get(v.kind, [])
			items.append({
				'name'                : v.name or k,
				'id'                  : k,
				'argumentDescription' : v.description,
			})
			self._effects[v.kind] = items

		self._load_settings()

	def _load_settings(self):
		try:
			with open(Settings.settings_file, 'r') as f:
				saved = json.load(f)
		except:
			saved = {}

		# Load global settings
		self.post_settings(saved.get('settings', {}))

		# Load per effect settings
		all_effects = saved.get('effects', {})

		for i in Settings.kinds:
			avail = self._effects.get(i, [])
			effects = all_effects.get(i, [])
			for v in effects:
				if not isinstance(v, dict):
					continue

				cls = v.get('id', None)
				cur = next((x for x in avail if x['id'] == cls), None)
				if cur:
					# Only apply if saved setting still applies
					self._set_effect(cur, v, False)

		# Get saved active effects
		active = saved.get('active', {})

		# Set active if they still apply, otherwise pick new default
		for i in Settings.kinds:
			opts = self._effects.get(i, [])
			val = active.get(i, None)
			if val and val in self._effects:
				self._active[i] = val
			elif opts:
				self._active[i] = opts[0]['id']

	def _set_effect(self, cur, data, save):
		obj = Effect.effects[cur['id']]
		cur['isSensorDriven'] = data.get('isSensorDriven', obj.isSensorDriven)
		cur['isScreenSaver'] = data.get('isScreenSaver', obj.isScreenSaver)
		cur['color'] = data.get('color', obj.color)
		cur['argument'] = data.get('argument', obj.argument)

	def _save_settings(self):
		obj = {
			'settings' : self._settings,
			'active'   : self._active,
			'effects'  : self._effects,
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

	def get_effect(self, kind):
		active = self._active.get(kind, None)
		avail = self._effects.get(kind, {})
		return next((x for x in avail if x['id'] == active), None)

	def set_effect(self, kind, data):
		pass
