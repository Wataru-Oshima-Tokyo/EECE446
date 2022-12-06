// Dylan Stark
// Kevin Douglass
// EECE 446
// Spring 2022

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <regex.h>
#include <netdb.h>

int sendall(int s, char *buf, int *len);
int recvall(int socketFd, char *buffer, int chunk_size);
int countOccurrences(char *strs, char *subs);

// Lab 2 code modified
int lookup_and_connect(const char *host, const char *service)
{

	struct addrinfo hints, *rp, *response;
	int socketFileDescriptor;

	/// Translate host address to IP_Address*/
	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_UNSPEC;	 // Server Agnostic | allow for both iPv4 and IPv6
	hints.ai_socktype = SOCK_STREAM; // Streaming Socket (Has Connection/Listens)
	hints.ai_flags = 0;				 // input flags
	hints.ai_protocol = 0;			 // 0 means TCP.

	// char node | e.g. www.example.com
	//  the Service being used | e.g. PORT # or 'Http'
	//  Address of addrinfo struct
	//  Store the (Socket Status) in Address of our result 'response'
	socketFileDescriptor = getaddrinfo(host, "80", &hints, &response);
	if (socketFileDescriptor != 0)
	{
		fprintf(stderr, "[Socket Status <ERROR>] \t getaddrinfo %s", gai_strerror(socketFileDescriptor));
		exit(1);
	} // if

	/* Iterate through the address list and try to connect
		- We also attempt to make sure we have a 'Valid' entry.
	*/
	for (rp = response; rp != NULL; rp = rp->ai_next)
	{

		// for each new connection, create new socket pid
		if ((socketFileDescriptor = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1)
		{
			continue;
		}

		// check if we have good connection
		if (connect(socketFileDescriptor, rp->ai_addr, rp->ai_addrlen) != -1)
		{
			break;
		}

		close(socketFileDescriptor); // close the socket
	}								 // for

	if (rp == NULL)
	{

		perror("[<Error> Client] onConnect:\n");
		return -1;
	}
	freeaddrinfo(response); // Free the linked list

	// Lookup IP and Connect to Server
	if ((socketFileDescriptor) < 0)
	{
		exit(1);
	}

	return socketFileDescriptor;
} // lookup_and_connect

int main(int argc, char *argv[])
{
	// Address
	int client_socket;
	const char *HOST_ADDR = "www.ecst.csuchico.edu";
	const char *PORT = "80";
	int CHUNK_SIZE;
	char *request = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";

	// passing args correct #
	if (argc != 2)
	{
		printf("Format: h1-counter [chunk size]\n");
		exit(1);
	}
	else if (argc == 2)
	{
		// Read in the argument
		CHUNK_SIZE = atoi(argv[1]);
	}

	char response[CHUNK_SIZE];

	if (CHUNK_SIZE <= 0 || CHUNK_SIZE > 1000)
	{
		printf("Error: Please enter CHUNK_SIZE > 0 and <= 1000\n");
		exit(1);
	}

	if ((client_socket = lookup_and_connect(HOST_ADDR, PORT)) < 0)
	{
		fprintf(stderr, "Error connecting %s\n", gai_strerror(client_socket));
		exit(1);
	}

	// Send request
	int byteCount = 0;
	int requestLen = strlen(request);
	if (sendall(client_socket, request, &requestLen) == -1)
	{
		fprintf(stderr, "error in send %s\n", gai_strerror(byteCount));
		exit(1);
	}

	// Bytes Recieved
	int total = 0;

	int tags = 0;	  /* Number of tags found*/
	int numBytes = 0; /* Number of Bytes returned from recvall (that calls recv) */
	while ((numBytes = partial_recv(client_socket, response, sizeof(response))) > 0)
	{
		total += numBytes;
		response[numBytes] = '\0';
		tags += countOccurrences(response, "<h1>");
	} // while

	if (numBytes < 0)
	{
		fprintf(stderr, "error in response %s\n", gai_strerror(numBytes));
		exit(1);
	}

	/* RESULTs */
	printf("Number of <h1> tags: %d\n", tags);
	printf("Number of bytes: %d\n", total);

	close(client_socket);
	return 0;
}

// Similar to Beej's Guide to Network Programming (modified to fit our needs)
int recvall(int socketFd, char *buffer, int chunk_size)
{

	int total = 0;			 // total # bytes sent
	int extras = chunk_size; // any extra bytes to send?
	int num_bytes = 0;		 // how many bytes we recieved
	while (total < chunk_size)
	{
		num_bytes = recv(socketFd, buffer + total, extras, 0);

		if (num_bytes <= 0)
		{
			break;
		}
		total += num_bytes;
		extras -= num_bytes;
	}

	return num_bytes == -1 ? -1 : total; // return -1 on failure, 0 on success
}

// Similar to Beej Guide to Network Programming (https://beej.us/guide/bgnet/)
int sendall(int s, char *buf, int *len)
{
	int total = 0;		  // how many bytes we've sent
	int bytesleft = *len; // how many we have left to send
	int n;

	while (total < *len)
	{
		n = send(s, buf + total, bytesleft, 0);
		if (n <= 0)
		{
			break;
		}
		total += n;
		bytesleft -= n;
	}

	*len = total;			 // return number actually sent here
	return n == -1 ? -1 : 0; // return -1 on failure, 0 on success
}

int countOccurrences(char *strs, char *subs)
{
	int count = 0;
	char *p1 = strstr(strs, subs);
	while (p1)
	{
		count++;
		p1 = strstr(p1 + 1, subs);
	}
	return count;
}
