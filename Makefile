all:
	python ../../libraries/nanopb/generator/nanopb_generator.py protocol/messages.desc -f protocol/Messages.options
