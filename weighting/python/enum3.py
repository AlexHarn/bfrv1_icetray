
from .enum import baseEnum, metaEnum

class enum(baseEnum, metaclass=metaEnum):
	"""This class mimicks the interface of boost-python-wrapped enums.
	
Inherit from this class to construct enumerated types that can
be passed to the I3Datatype, e.g.:

	class DummyEnummy(tableio.enum):
		Foo = 0
		Bar = 1
		Baz = 2

	desc = tableio.I3TableRowDescription()
	desc.add_field('dummy', tableio.I3Datatype(DummyEnummy), '', '')
"""
