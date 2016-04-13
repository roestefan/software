from .interface import * 

class Memory(object):
	modid = None

	def __init__(self, mod):
		self.modid = mod

	def loadelf(self, filename):
		return python_osd_mem_loadelf(self.modid, filename)

    
class Session(object):
	memories = []

	def __init__(self):
		for m in python_osd_get_memories():
			mem = Memory(m)
			self.memories.append(mem)

	def reset(self, halt=False):
		python_osd_reset(halt)
	
	def get_memories(self):
		return self.memories

	def start(self):
		python_osd_start()
		
	def wait(self, secs):
		python_osd_wait(secs)