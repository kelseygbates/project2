OBJS = coord_traffic.cpp
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

coord_traffic : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o coord_traffic -pthread

coord_traffic.o :
	$(CC) $(CFLAGS) coord_traffic.cpp

clean:
	\rm *.o *~