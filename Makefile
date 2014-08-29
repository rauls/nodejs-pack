BIN = ./node_modules/.bin
MOCHA_OPTS = --timeout 10s --recursive
REPORTER = spec
TEST_FILES = test/acceptance

all:
	#$(MAKE) static
	node-gyp configure build

clean:
	rm -rf build

lint:
	$(BIN)/jshint hipack.js test/*
	
temp:
	rm -rf tmp/hipack
	mkdir -p tmp/hipack
	cp -r README *.{cc,h,js*} binding.gyp Makefile test tmp/hipack
	cd tmp/hipack

test:
	node test/test.js
	
install:
	@npm install

package: temp install
	cd tmp && tar -czvf hipack.tgz hipack
	@npm hipack

