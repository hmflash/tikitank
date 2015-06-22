effects = {}

TREADS = 'treads'
BARREL = 'barrel'
PANELS = 'panels'

class store_effect_type(type):
	def __init__(cls, name, bases, dict):
		super(store_effect_type, cls).__init__(name, bases, dict)
		name = cls.__name__
		if name != 'evil' and name != 'Effect':
			global effects
			effects[name] = cls

evil = store_effect_type('evil', (object,), {})

class Effect(evil):

	kind = None
	isSensorDriven = False

	def __init__(self, *k, **kw):
		pass
