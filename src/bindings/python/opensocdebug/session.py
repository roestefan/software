from .interface import * 

class Module(object):
	modid = None
	type = None
	def __init__(self, id, type):
		self.modid = id
		self.type = type
	def create(id, type):
		if type == "MAM":
			return Memory(id)
		else:
			return Module(id, type)
	create = staticmethod(create)

	def get_type(self):
		return self.type
	def get_id(self):
		return self.modid

class Memory(Module):
	def __init__(self, id):
		super(Memory, self).__init__(id, "MAM")

	def loadelf(self, filename):
		return python_osd_mem_loadelf(self.modid, filename)

    
class Session(object):
	memories = []
	modules = []

	def __init__(self):
		for i in range(0,python_osd_get_num_modules()):
			m = Module.create(i, python_osd_get_module_name(i))
			self.modules.append(m)

	def reset(self, halt=False):
		python_osd_reset(halt)
	
	def get_memories(self):
		return self.memories

	def get_modules(self,type=None):
		if type == None:
			return self.modules
		else:
			mods = []
			for m in self.modules:
				if m.get_type() == type:
					mods.append(m)
			return mods

	def start(self):
		python_osd_start()
		
	def wait(self, secs):
		python_osd_wait(secs)