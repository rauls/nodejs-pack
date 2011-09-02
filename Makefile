all:
	#$(MAKE) static
	node-waf configure build

clean:
	rm -rf build

temp:
	rm -rf tmp/pack
	mkdir -p tmp/pack
	cp -r README *.{cc,h,js*} wscript Makefile deps test tmp/pack
	cd tmp/pack && rm -rf deps/*/.git* deps/*/*.o deps/pack/libpack.*

package: temp
	cd tmp && tar -czvf pack.tgz pack
