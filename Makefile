default_target:
	cmake . -B build && cd build && make

differentiator: default_target
	cd build && ./differentiator --help

test: default_target
	cd build && ./tests

clear:
	rm -rf ./build