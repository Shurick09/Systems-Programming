all: bankingClient bankingServer

bankingClient: bankingClient.c
	gcc -g -Werror -fsanitize=address -pthread bankingClient.c -o bankingClient

bankingServer: bankingServer.c
	gcc -g -Werror -fsanitize=address -pthread bankingServer.c -o bankingServer

clean:
	rm bankingClient bankingServer
