from tank import Effect

class RainbowTread(Effect.Effect):

	name = "Rainbow Tread"
	kind = Effect.TREADS
	isSensorDriven = True

	def __init__(self, *k, **kw):
		super(RainbowTread, self).__init__(*k, **kw)
