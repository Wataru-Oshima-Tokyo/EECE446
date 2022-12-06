// This assignment is done by using the code from lab 2.
//Authors
//Oshima, Wataru
//Chu, Chen
//
//EECE446
//Spring 2022

/* This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include<unistd.h>	//usleep
#include<fcntl.h>	//fcntl

#define HOST "www.ecst.csuchico.edu"
#define PORT "80"
#define CHUNK_SIZE 512
long bytes_per_chunk = 20;
int num=0;
/*
 * Lookup a host IP address and connect to it using service. Arguments match the first two
 * arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible for closing
 * the returned socket.
 */
int lookup_and_connect( const char *host, const char *service );

int recv_timeout(int s , int timeout)
{
	int size_recv , total_size= 0;
	struct timeval begin , now;
	char chunk[CHUNK_SIZE];
	double timediff;
	int flag = 0;
	//make socket non blocking
	// fcntl(s, F_SETFL, O_NONBLOCK);
	
	//beginning time
	gettimeofday(&begin , NULL);
	
	while(1)
	{
		gettimeofday(&now , NULL);
		
		//time elapsed in seconds
		timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
		
		//if you got some data, then break after timeout
		if( total_size > 0 && timediff > timeout )
		{
			break;
		}
		
		//if you got no data at all, wait a little longer, twice the timeout
		else if( timediff > timeout*2)
		{
			break;
		}
		

		memset(chunk ,0 , bytes_per_chunk);	//clear the variable
		if((size_recv =  recv(s , chunk , bytes_per_chunk , 0) ) < 0)
		{
			//if nothing was received then we want to wait a little before trying again, 0.1 seconds
			// usleep(100000);
			usleep(100000);
			if(flag){
				break;
			}
			flag = 1;
		}
		else
		{
			int len = strlen(chunk);
			total_size += size_recv;
			// printf("%d", total_size);
			
			// make a copy of the chunk so that 
			const char *tmp = chunk;
			//once you found the h1 tag then go to the while loop to find how many h1 tas are in the chunk
			if  (strstr(tmp, "<h1>")){
				// num++;
				tmp++;
				while((tmp =strstr(tmp, "<h1>"))){
					num++;
					tmp++; // =strlen("<h1>");
				}
				// printf("%s" , chunk);
				// printf("\n");

			  /*I am not sure but sometimes recv() hangs though it sends all the data 
			  	so it termminates once the total byte size reachs 4547 which is the 
				  total size of html   
			  */
			}
			else if (strstr(chunk, "</html>") || total_size>=4547){
				break;
			}
			gettimeofday(&begin , NULL);
		}
	}
	
	return total_size;
}




int main(int argc, char *argv[]) {
	int s;
	/* Lookup IP and connect to server */
	if ( ( s = lookup_and_connect(HOST, PORT) ) < 0 ) {
		exit( 1 );
	}
	// printf("connection ok\n");

	/* Modify the program so it
	 *
	 * 1) connects to www.ecst.csuchico.edu on port 80 (mostly done above)
	 * 2) sends "GET /~kkredo/file.html HTTP/1.0\r\n\r\n" to the server
	 * 3) receives all the data sent by the server (HINT: "orderly shutdown" in recv(2))
	 * 4) prints the total number of bytes received
	 *
	 * */

	char *msg = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";
	if(argc==2){
		sscanf(argv[1], "%ld", &bytes_per_chunk); //https://stackoverflow.com/questions/9748393/how-can-i-get-argv-as-int/42860313
		if (bytes_per_chunk>521 || bytes_per_chunk <10){
			printf("please put a number more than 10 and less than 521\n");
			exit(1);
		}
		// bytes_per_chunk = (int)argv[1];

	}
    int len = strlen(msg);	 
	if(send(s, msg, len, 0) < 0)
		printf("error on send");
	int total_recv = recv_timeout(s,3);
	printf("Number of <h1> tags: %d\n", num);
	printf( "Number of bytes: %d\n", total_recv);

	close(s);

	return 0;
}

int lookup_and_connect( const char *host, const char *service ) {
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	int s;

	/* Translate host name into peer's IP address */
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if ( ( s = getaddrinfo( host, service, &hints, &result ) ) != 0 ) {
		fprintf( stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror( s ) );
		return -1;
	}

	/* Iterate through the address list and try to connect */
	for ( rp = result; rp != NULL; rp = rp->ai_next ) {
		if ( ( s = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol ) ) == -1 ) {
			continue;
		}

		if ( connect( s, rp->ai_addr, rp->ai_addrlen ) != -1 ) {
			break;
		}

		close( s );
	}
	if ( rp == NULL ) {
		perror( "stream-talk-client: connect" );
		return -1;
	}
	freeaddrinfo( result );

	return s;
}