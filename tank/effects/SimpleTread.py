from tank import Effect

class SimpleTread(Effect.Effect):

	name = "Simple Tread"
	kind = Effect.TREADS
	isSensorDriven = True

	def __init__(self, *k, **kw):
		super(SimpleTread, self).__init__(*k, **kw)
