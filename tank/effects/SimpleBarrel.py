from tank import Effect

class SimpleBarrel(Effect.Effect):

	name = "Simple Barrel"
	kind = Effect.BARREL
	isScreenSaver = True
	description = 'Speed'
	argument = '100'

	def __init__(self, *k, **kw):
		super(SimpleBarrel).__init__(*k, **kw)
