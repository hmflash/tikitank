from tank import Effect

class SimpleBarrel(Effect.Effect):
	"""Simple Barrel"""

	kind = Effect.BARREL

	def __init__(self, *k, **kw):
		super(SimpleBarrel).__init__(*k, **kw)
