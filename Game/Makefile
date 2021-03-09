CC=gcc
CFLAGS=-I.
DEPS = alienControl.h alienBullets.h button_handler.h gpio_button.h imageRender.h intc.h game_over_splash.h player.h saucer.h bullet.h bunker.h globals.h audioPlayer.h wav.h audio_adau1761.h uio.h i2cps.h gpio_switches.h
OBJ =  alienControl.o alienBullets.o button_handler.o gpio_button.o imageRender.o intc.o game_over_splash.o player.o saucer.o bullet.o bunker.o globals.o audioPlayer.o wav.o audio_adau1761.o uio.o i2cps.o gpio_switches.o

ODIR=obj

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)


AlienInvasion: main.c $(OBJ)
	$(CC) -o AlienInvasion $^ $(CFLAGS)


clean:
		rm -f $(ODIR)/*.o
