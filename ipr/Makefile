all:
	# [ -d "build" ] && rm -rf build
	[ -d "build" ] || mkdir build
	cp src/install.sh build
	tar czf build/node_modules.tgz src/node_modules
	cp -rfL src/bin build/bin
	cp -rfL src/etc build/etc
	bash tools/minify.sh web
	bash tools/minify.sh ipr
