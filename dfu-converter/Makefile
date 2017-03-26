all: main.c
	$(CC) main.c -o hex2dfuse

upload: all
	$(MAKE) -C ../src hex
	./hex2dfuse ../src/main.hex ../src/main.dfu
	sudo dfu-util -a 0 -D ../src/main.dfu
