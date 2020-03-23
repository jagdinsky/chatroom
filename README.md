## This program is a chatroom

The program runs in the terminal.

You need to download client.c and Makefile,
compile client file (enter "make client" in the terminal)
and execute with your server's port:
```
./client <server's ip-address> <port>
```
Then you can chat with the other users online on your chosen server.

You can also start your own server.
In this case you need to download server.c and Makefile,
compile server file (enter "make server" in the terminal)
and execute the program this way ("port" is an arbitrary number 1000-10000):

```
./server <port>
```

To exit client send "-exit" in your terminal.
To exit server press Ctrl+C.
