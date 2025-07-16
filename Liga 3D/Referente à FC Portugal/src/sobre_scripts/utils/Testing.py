from abc import ABC, abstractmethod

class Testing(ABC):
	"""
	Descrição:
		Apenas uma classe para organizarmos os testes de maneira mais inteligente.
	"""

	@abstractmethod
	def execute():
		pass

