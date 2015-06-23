from tank import Effect

class SolidPanels(Effect.Effect):

	name = "Solid Colors"
	kind = Effect.PANELS
	isScreenSaver = True


	def __init__(self, *k, **kw):
		super(SolidPanels, self).__init__(*k, **kw)
