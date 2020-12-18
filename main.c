
/* Lenny Orengo 2020...win32 support...
I was able to create a socket, print the challenge description and hex decode the compressed string. I however experienced challenges
gzip decompressing the decoded string */

#include<stdio.h>
#include<winsock2.h>                            //Winsock library header file
#include<stdlib.h>
#pragma comment(lib,"ws2_32.lib")               //Winsock library
#include<string.h>
#include <assert.h>
#include "zlib.h"                               //zlib header file
//--------------------------------------------------------------------------------------------
void print_hex(unsigned char *s, size_t n)
 {                                                              //function to output the decimal result
	int i;
	for (i=0; i<n; ++i)
		printf("%d", (unsigned int)s[i]);
	printf("\n");
}

void hextobin(unsigned char *v, unsigned char *s, size_t n)     //function to convert hexadecimal to bin
 {
	int i;
	char _t[3];
	unsigned char *p = s;
	for (i=0; i<n; ++i) {
		memcpy(_t, p, 2);
		_t[2] = '\0';
		v[i] = (int)strtol(_t, NULL, 16);
		p += 2;
	}
}

//--------------------------------------------------------------------------------------------

int main(int argc , char *argv[])
{

	WSADATA wsa;
	SOCKET s;
	struct sockaddr_in server;
	char   message,server_reply[348];                   //Buffer size
	int recv_size;

	if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
	{
		printf("Failed. Error Code : %d",WSAGetLastError());
		return 1;
	}

//---------------Create socket---------------------------------------------------------------


	if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)          //IPV-4
	{
		printf("Could not create socket : %d" , WSAGetLastError());
	}

	printf("Socket created.\n");



	server.sin_addr.s_addr = inet_addr("51.11.50.70");                      //IP-Address
	server.sin_family = AF_INET;
	server.sin_port = htons( 5050 );                                        //Port number


//-----------------------Connect to server----------------------------------------------------------------
	if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connection error");
		return 1;
	}

	puts("Connected to Challenge server");
	printf("-------------------------------------------------------------------------\n");                                                  //Connection successful

//-----------------------Receive data from server-----------------------------------------------------------
    int count = 0;
    int total = 0;

while ((count = recv(s, &server_reply[total], sizeof (server_reply) - total, 0)) > 0)
{
    total += count;

}
if (count == -1)
{
    perror("receive error");
}

//---------------Format and display challenge description----------------------------------------------------------
    server_reply[348] = '\0';
    printf("Challenge Description:\n");
    printf("%s\n\n",server_reply);
    printf("-------------------------------------------------------------------------\n");

//----------Store the string in file and extract the string to be decoded and decompressed---------------------------------------------------------------------------------------

    FILE *fp;
    fp = fopen("Challenge.txt","w+");
    fputs(server_reply,fp);
    fclose(fp);

    char c[1000];
    FILE *fptr;
    fptr = fopen("Challenge.txt", "r");
    fscanf(fptr, "%*[^\n] %*[^\n] %*[^\n] %*[^\n] %[^\n]", c);
    printf("Hex string to be decompressed:\n%s\n", c);
    printf("-------------------------------------------------------------------------\n");
    fclose(fptr);


//--------------Hex decode the string---------------------------------------------------------------


    int n = strlen(c);
	char v[n/2];

	hextobin(v, c, n/2);
	printf("\nHex decoded string:\n");
	print_hex(v, n/2);
    printf("-------------------------------------------------------------------------\n");
//------------------Decompression-----------------------------------------------------------------------

    char b[500];                                                    //output char array
    z_stream infstream;                                              // zlib struct
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
                                                                    //  "v" is the input and "b" is the compressed output
    infstream.avail_in =  (uInt)strlen(v)+1;                         // size of input
    infstream.next_in = (Bytef *)v;                                 // input char array
    infstream.avail_out = (uInt)sizeof(b);                           // size of output
    infstream.next_out = (Bytef *)b;                                // output char array

                                                                    // Decompression
    inflateInit(&infstream);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);

//----------------Send decompressed string to server---------------------------------------------------------------------------

	send(s ,b , strlen(b) , 0);
	puts("Decompressed string sent for validation\n");

//--------------------Receive reply from server---------------------------------------------------------------------------------
    if((recv_size = recv(s , server_reply , 2000 , 0)) == SOCKET_ERROR)
	{
		puts("receive failed");
	}

	puts("Reply received:\n");

                                                //Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';
	puts(server_reply);
    return 0;
}
