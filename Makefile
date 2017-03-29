fanalert.so: fanalert.c
	gcc -Wall `pkg-config --cflags gtk+-2.0 lxpanel` -g -shared -fPIC fanalert.c -o fanalert.so `pkg-config --libs lxpanel`

install: fanalert.so
	cp fanalert.so `pkg-config --variable=pluginsdir lxpanel`
