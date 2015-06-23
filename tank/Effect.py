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

	name = None            # Pretty name for effect
	kind = None            # What kind of effect is it?
	isSensorDriven = False # Drive the effect from the VSS?
	isScreenSaver = False  # Include in screen saver by default?
	description = None     # Description of argument
	argument = None        # Default argument value
	color = '#ffffff'      # Default color for the effect

	def __init__(self, *k, **kw):
		pass
