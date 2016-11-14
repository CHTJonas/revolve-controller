arduino-builder \
	-compile \
	-fqbn=arduino:avr:mega:cpu=atmega2560 \
	-tools /usr/local/bin \
	-libraries ./libraries \
	-hardware ~/Devel/Arduino/hardware \
	-prefs=runtime.tools.avr-gcc.path=/usr/local \
	-prefs=runtime.tools.ctags.path=/usr/local/bin \
	-prefs=tools.ctags.path=/usr/local/bin \
	-prefs=tools.ctags.cmd.path=/usr/local/bin/ctags \
	-prefs=tools.ctags.pattern='"{cmd.path}" -u --language-force=c++ -f - --c++-kinds=svpf --fields=KSTtzns --line-directives "{source_file}"' \
	-build-path `pwd`/build \
	$@ \
	revolve_controller.ino
