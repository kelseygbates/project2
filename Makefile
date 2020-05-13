OBJS = coord_traffic.cpp
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

coord_traffic : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) main.cpp -o coord_traffic

coord_traffic.o :
	$(CC) $(CFLAGS) coord_traffic.cpp

clean:
	\rm *.o *~