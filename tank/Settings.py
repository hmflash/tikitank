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
			for cur in avail:
				cls = cur.get('id', None)

				new = next((x for x in effects if x['id'] == cls), None)
				if not isinstance(new, dict):
					new = {}

				# Always initialize effect settings
				self._set_effect(cur, new, False, False)

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

	def _set_effect(self, cur, data, save, need_save):
		obj = Effect.effects[cur['id']]

		need_save |= self._apply_effect('isSensorDriven', cur, data, obj)
		need_save |= self._apply_effect('isScreenSaver', cur, data, obj)
		need_save |= self._apply_effect('color', cur, data, obj)

		if obj.description:
			need_save |= self._apply_effect('argument', cur, data, obj)

		if save and need_save:
			self._save_settings()

	def _apply_effect(self, name, cur, data, obj):
		val = cur.get(name, None)
		new = data.get(name, None)

		if str(new) == 'true':
			new = True
		elif str(new) == 'false':
			new = False

		if not new is None and new != val:
			cur[name] = new
			return True
		elif not val is None:
			cur[name] = val
		else:
			cur[name] = getattr(obj, name)

		return False

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

	def _get_effect(self, kind, id):
		avail = self._effects.get(kind, {})
		return next((x for x in avail if x['id'] == id), {})

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
		return self._get_effect(kind, active)

	def set_effect(self, kind, data):
		cur = self._active.get(kind, None)
		new = data.get('id', cur)
		e = self._get_effect(kind, new)
		if e:
			self._active[kind] = new
			self._set_effect(e, data, True, new != cur)
