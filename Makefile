all:
	protoc proto/* -I proto -o messages.desc
	python ~/WICED_for_EMW3162/nanopb-0.3.3-linux-x86/generator/nanopb_generator.py messages.desc 
