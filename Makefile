
all: CLIENT LOCAL_DNS DNS

CC = gcc
LD = gcc

#CFLAGS = -g -Wall -Iinclude
CFLAGS += -std=gnu9x
LDFLAGS = 

LIBS = -lpthread

HDRS = ./*.h

SRCS = client.c dns.c local_dns.c

OBJS = $(patsubst %.c,%.o,$(SRCS))

$(OBJS) : %.o : %.c ./*.h
	$(CC) -c $(CFLAGS) $< -o $@

CLIENT: client.o
	$(LD) $(LDFLAGS) client.o -o client 

LOCAL_DNS: local_dns.o
	$(LD) $(LDFLAGS) local_dns.o -o local_dns $(LIBS)
	
DNS: dns.o
	$(LD) $(LDFLAGS) dns.o -o dns
	
clean:
	rm -f *.o client local_dns dns

tags: $(SRCS) $(HDRS)
	ctags $(SRCS) $(HDRS)
