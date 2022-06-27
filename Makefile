# PROGRAMACAO II - Trabalho II: Jogo Grafico (Boulder Dash)
# Feito por Gabriel de Oliveira Pontarolo      GRR20203895
# Makefile

CFLAGS = --std=c99 -Wall 
LDFLAGS = `pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5 allegro_audio-5 allegro_acodec-5 allegro_image-5`
objects = main.o boulder-dash.o game-states.o

# default
all: boulder-dash

# ligacao
boulder-dash: $(objects)
	cc -o boulder-dash $(objects) $(LDFLAGS)	

# compilacao
main.o: main.c boulder-dash.h
boulder-dash.o: boulder-dash.c boulder-dash.h
game-states.o: game-states.c game-states.h boulder-dash.h

# depuracao
debug: CFLAGS += -DDEBUG -g
debug: all

# remove arquivos temporarios
clean:
	-rm -f $(objects) *~

# remove tudo que nao for codigo fonte
purge: clean
	-rm -f boulder-dash