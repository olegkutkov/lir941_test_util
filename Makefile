
#
# Kutkov Oleg (kutkov.o@yandex.ru) (July 2017)
#

CC := gcc
PROGRAM = test_client
SRC := main.c
CFLAGS := -Wall
LDFLAG := -lm

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAG) -o $(PROGRAM)

clean:
	rm -fr $(PROGRAM) $(PROGRAM).o

