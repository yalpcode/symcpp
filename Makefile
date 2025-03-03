default_target:
	cmake . -B build && cd build && make

main:
	cd build && ./main

test:
	cd build && ./mathtest

clear:
	rm -rf ./build