//#define BUFFER_SIZE 1024
#define SOCKET_ERROR -1

int make_send_socket(int port_num, int &hSocket)
{
   int nHostPort=port_num;
  struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char * strHostName=(char*)"dozer";
   // int nHostPort=5555;
  //  FILE* command_fp = fopen("test.txt", "wb");
    int  curr_index=0;
    //if (NULL == command_fp){
    //  fprintf(stderr, "unable to open test.txt\n");
    //  exit(-1);
    //}

   hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if(hSocket == SOCKET_ERROR)
    {
        printf("\nCould not make a socket\n");
        //return 0;
    }

    /* get IP address from name */
    pHostInfo=gethostbyname(strHostName);
    /* copy address into long */
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=port_num;
    Address.sin_family=AF_INET;

    printf("\nConnecting to %s on port %d\n",strHostName,nHostPort);

    /* connect to host */
    if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) 
       == SOCKET_ERROR)
    {
        printf("\nCould not connect to host\n");
        //return 0;
    }
}
