from tank import Effect

class RainbowTread(Effect.Effect):
	"""Rainbow Tread"""

	kind = Effect.TREADS
        isSensorDriven = True

	def __init__(self, *k, **kw):
		super(RainbowTread).__init__(*k, **kw)
