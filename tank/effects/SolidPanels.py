from tank import Effect

class SolidPanels(Effect.Effect):
	"""Solid Colors"""

	kind = Effect.PANELS

	def __init__(self, *k, **kw):
		super(SolidPanels).__init__(*k, **kw)
