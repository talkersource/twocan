CC=gcc
WARNS=-W -Wall -Wundef -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings -Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Winline -O0 -ggdb3
CFLAGS=`gnome-config --cflags timer_q` $(WARNS)
LDFLAGS=`gnome-config --libs timer_q`


all: ex1 ex2


ex1: ex1.c
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

ex2: ex2.c
		$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<
