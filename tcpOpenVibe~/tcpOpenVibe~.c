/*
Avidabits, engineer and artist wrote this code
   http://avidabits.tumblr.com/
   https://github.com/Avidabits
   https://www.linkedin.com/in/bernardez/
   mail: avidabits@gmail.com rosa.bernardez@gmail.com

This external for PureData is intented to connect with the signal from 
OpenVibe in order to process biological signals in PureData in Windows

Ir order to execute que external you neeed to have 2 dll
 -tcpOpenVibe~.dll (this very code)
 -pthreadVC2.dll that is part of PTHREADS-WIN32 describe below

With code from
 * HOWTO write an External for Pure data
 * (c) 2001-2006 IOhannes m zmölnig zmoelnig
an With most code from 
 * tcpclient.c Martin Peach 20060508, working version 20060512 
 * linux version 20060515 

---------------------------------------------------------
This Pd External is intented to receive data from OpenVibe TCPWrite
taking into account the specification described at:
http://openvibe.inria.fr//documentation/2.0.1/Doc_BoxAlgorithm_TCPWriter.html

In order to use the POSIX sockets of tcpclient I use the library: 
PTHREADS-WIN32 RELEASE 2.9.0 (2012-05-25)
-----------------------------------------
Web Site: http://sourceware.org/pthreads-win32/
FTP Site: ftp://sourceware.org/pub/pthreads-win32
Maintainer: Ross Johnson <ross.johnson@loungebythelake.net>

---------------- TODO LIST 18-May-2018
* - Support for one signal channel only, but OpenVibe TCPWriter supports several channels
* - Isuue related to TCPWriter header. It seems that two first the fiels in header 
   (format and endiness) are not properly documented, I get the same number despite 
   I revised the documentation and code carefully
* - Isuue related to different float managment in PureData and OpenVibe. 
    OpenVibe sample are 64 bits float (double) and PureData floats are 32 bits. 
    Loss of precission is not a big isuue for the application, whatever the case
    Hope this issue will be solved with Pd for 64 bits.
*/


/**
 * include the interface to Pd 
 */
#include "m_pd.h"
#include "s_stuff.h"

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#ifndef _WIN32
#include <sys/socket.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#else
#include <winsock2.h>
#include <ws2tcpip.h> /* for socklen_t */
#endif

/* support older Pd versions without sys_open(), sys_fopen(), sys_fclose() */
#if PD_MAJOR_VERSION == 0 && PD_MINOR_VERSION < 44
#define sys_open open
#define sys_fopen fopen
#define sys_fclose fclose
#endif


#ifdef _MSC_VER
#define snprintf sprintf_s
#endif

#define DEFPOLLTIME 20  /* check for input every 20 ms */
#define MAX_TCPCLIENT_SEND_BUF 65536L // longer than data in maximum UDP packet
/* each send is handled by a new thread with a new parameter struct: */
/* these are stored under x->x_tsp[0..MAX_TCPCLIENT_THREADS-1] */
/* The buffer is preallocated for speed. */
typedef struct _tcpclient_sender_params
{
    char                x_sendbuf[MAX_TCPCLIENT_SEND_BUF]; /* possibly allocate this dynamically for space over speed */
    size_t              x_buf_len;
    int                 x_sendresult;
    pthread_t           sendthreadid;
    int                 threadisvalid; /*  non-zero if sendthreadid is an active thread */
    struct _tcpOpenVibe_tilde   *x_x;
} t_tcpclient_sender_params;
#define MAX_TCPCLIENT_THREADS 32
/* MAX_TCPCLIENT_THREADS is small to avoid wasting space. This is the maximum number of concurrent threads. */

/**
 * define a new "class" 
 */
static char objName[] = "tcpOpenVibe~";
static t_class *tcpOpenVibe_tilde_class;


/**
 * this is the dataspace of our new object
 * we don't need to store anything,
 * however the first (and only) entry in this struct
 * is mandatory and of type "t_object"
 */
typedef struct _tcpOpenVibe_tilde {
  //primer campo comun
  t_object  x_obj;
  // zona tcpclient
    t_clock                     *x_clock;
    t_clock                     *x_poll;
    t_clock                     *x_sendclock;
    t_outlet                    *x_signalout;//
    t_outlet                    *x_addrout;
    t_outlet                    *x_connectout;
    t_outlet                    *x_statusout;
    int                         x_dump; // 1 = hexdump received bytes
    int                         x_verbosity; // 1 = post connection state changes to main window
    int                         x_fd; // the socket
    int                         x_fdbuf; // the socket's buffer size
    char                        *x_hostname; // address we want to connect to as text
    int                         x_connectstate; // 0 = not connected, 1 = connected

    int                         x_port; // port we're connected to
    uint32_t                    x_addr; // address we're connected to as 32bit int
    t_atom                      x_addrbytes[4]; // address we're connected to as 4 bytes
    /*t_atom*/ t_sample         x_msgoutbuf[MAX_TCPCLIENT_SEND_BUF]; // received data as float atoms
    int                         x_msgout_from;//The socket reader will put here the number
                                              // of bytes available for the dsp to process from
    
    int                         x_msgout_to;//The socket reader will put here the number
                                              // of bytes available for the dsp to process to

    unsigned char               x_msginbuf[MAX_TCPCLIENT_SEND_BUF]; // received data as bytes
    char                        *x_sendbuf; // pointer to data to send
    int                         x_sendbuf_len; // number of bytes in sendbuf
    int                         x_sendresult;
    int                         x_blocked;
    /* multithread stuff */
    pthread_t                   x_threadid; /* id of connector child thread */
    pthread_attr_t              x_threadattr; /* attributes of connector child thread */
    pthread_attr_t              x_sendthreadattr; /* attributes of all sender child thread for sending */
    int                         x_nextthread; /* next unused x_tsp */
    t_tcpclient_sender_params   x_tsp[MAX_TCPCLIENT_THREADS];
/* Thread params are used round-robin to avoid overwriting buffers when doing multiple sends */
    /* OpenVibe stuff*/
    int                        x_ovh_read; // false means that still didnt read the OpenVibe header
} t_tcpOpenVibe_tilde;

//// ZONA DE TCPCLIENT EXCEPTO FREE, NEW Y SETUP QUE ES OBLIGATORIO FUSIONAR///
static void tcpclient_verbosity(t_tcpOpenVibe_tilde *x, t_float verbosity);
static void tcpclient_dump(t_tcpOpenVibe_tilde *x, t_float dump);
static void tcp_client_hexdump(t_tcpOpenVibe_tilde *x, long len);
static void tcpclient_tick(t_tcpOpenVibe_tilde *x);
static void *tcpclient_child_connect(void *w);
static void tcpclient_connect(t_tcpOpenVibe_tilde *x, t_symbol *hostname, t_floatarg fportno);
static void tcpclient_disconnect(t_tcpOpenVibe_tilde *x);
static void tcpclient_send(t_tcpOpenVibe_tilde *x, t_symbol *s, int argc, t_atom *argv);
static int tcpclient_send_buf(t_tcpOpenVibe_tilde *x, char *buf, int buf_len);
static void *tcpclient_child_send(void *w);
static void tcpclient_sent(t_tcpOpenVibe_tilde *x);
static int tcpclient_get_socket_send_buf_size(t_tcpOpenVibe_tilde *x);
static int tcpclient_set_socket_send_buf_size(t_tcpOpenVibe_tilde *x, int size);
static void tcpclient_buf_size(t_tcpOpenVibe_tilde *x, t_symbol *s, int argc, t_atom *argv);
static void tcpclient_rcv(t_tcpOpenVibe_tilde *x);
static void tcpclient_poll(t_tcpOpenVibe_tilde *x);
static void tcpclient_unblock(t_tcpOpenVibe_tilde *x);

// las funciones de construccion, destruccion y configuracion
__declspec(dllexport) void tcpOpenVibe_tilde_free(t_tcpOpenVibe_tilde *x);
__declspec(dllexport) void tcpOpenVibe_tilde_dsp(t_tcpOpenVibe_tilde *x, t_signal **sp);

// OpenVibe related declarations
#define SIZE_OPENVIBE_HEADER 32
#define SIZE_OPENVIBE_SAMPLE 8

typedef struct _OpenVibe_header{
    // ATENCION: solo válido para 32 bits
  unsigned __int32 format_version;
  unsigned __int32 endianess;
  unsigned __int32 sampling_frecuency;
  unsigned __int32 n_channels;
  unsigned __int32 samples_per_chunk;
  unsigned __int32 reserved0;
  unsigned __int32 reserved1;
  unsigned __int32 reserved2;
}t_OpenVibe_header;

static double bytes2double(unsigned char *c);
static t_OpenVibe_header OpenVibe_header_from_bytes(char *bytes);
void post_OpenVibeHeader(t_OpenVibe_header *ovh);

CRITICAL_SECTION cs; //los bytes leidos se acceden en threads diferentes 
                     // por lo que usare una seccion critica para editar 
                     // y modificar los bytes leidos en el socket


t_sample sample_fake_triangular()
{
static t_sample fake_sample=0;
static int subiendo=0;

      if (fake_sample >=1.0) subiendo=0;
      else if (fake_sample<-1.0) subiendo=1;
      if (subiendo) fake_sample+=0.1;
      else fake_sample-=0.1;

return fake_sample;

}
static void tcpclient_dump(t_tcpOpenVibe_tilde *x, t_float dump)
{
    x->x_dump = (dump == 0)?0:1;
}

static void tcpclient_verbosity(t_tcpOpenVibe_tilde *x, t_float verbosity)
{
    x->x_verbosity = (verbosity == 0)?0:1; /* only two states so far */
}

static void tcp_client_hexdump(t_tcpOpenVibe_tilde *x, long len)
{
#define BYTES_PER_LINE 16
    char            hexStr[(3*BYTES_PER_LINE)+1];
    char            ascStr[BYTES_PER_LINE+1];
    long            i, j, k = 0L;
    unsigned char   *buf = x->x_msginbuf;

    if (x->x_verbosity) post("%s_hexdump %d:", objName, len);
    while (k < len)
    {
        for (i = j = 0; i < BYTES_PER_LINE; ++i, ++k, j+=3)
        {
            if (k < len)
            {
                snprintf(&hexStr[j], 4, "%02X ", buf[k]);
                snprintf(&ascStr[i], 2, "%c", ((buf[k] >= 32) && (buf[k] <= 126))? buf[k]: '.');
            }
            else
            { // the last line
                snprintf(&hexStr[j], 4, "   ");
                snprintf(&ascStr[i], 2, " ");
            }
        }
        post ("%s%s", hexStr, ascStr);
    }
}

static void tcpclient_tick(t_tcpOpenVibe_tilde *x)
{
    outlet_float(x->x_connectout, 1);
}

static void *tcpclient_child_connect(void *w)
{
    t_tcpOpenVibe_tilde         *x = (t_tcpOpenVibe_tilde*) w;
    struct sockaddr_in  server;
    struct hostent      *hp;
    int                 sockfd;

    if (x->x_fd >= 0)
    {
        error("%s_child_connect: already connected", objName);
        return (x);
    }

    /* create a socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //AF_INET:ipv4, SOCK_STREAM:tcp, 0: defaul protocol for stream, tcp
#ifdef _DEBUG
    post("%s: send socket %d\n", objName, sockfd);
#endif
    if (sockfd < 0)
    {
        sys_sockerror("tcpclient: socket");
        return (x);
    }
    /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(x->x_hostname);
    if (hp == 0)
    {
        sys_sockerror("tcpclient: bad host?\n");
        return (x);
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

    /* assign client port number */
    server.sin_port = htons((u_short)x->x_port);

    if (x->x_verbosity) post("%s: connecting socket %d to port %d", objName, sockfd, x->x_port);
    /* try to connect */
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
        sys_sockerror("tcpclient: connecting stream socket");
        sys_closesocket(sockfd);
        return (x);
    }
    x->x_fd = sockfd;
    x->x_addr = ntohl(*(uint32_t *)hp->h_addr);
    /* outlet_float is not threadsafe ! */
    // outlet_float(x->x_obj.ob_outlet, 1);
    x->x_connectstate = 1;
    x->x_blocked = 0;
    x->x_ovh_read=0; // since we are just connected, still didnt read the OpenVibe header
    /* use callback instead to set outlet */
    clock_delay(x->x_clock, 0);
    return (x);
}

static void tcpclient_connect(t_tcpOpenVibe_tilde *x, t_symbol *hostname, t_floatarg fportno)
{
    /* if we are not already connected, as Ivica Ico Bukvic 5/5/10 <ico@bukvic.net> noted */
    if (0 != x->x_connectstate)
    {
        error("%s_connect: already connected to %s:%d on socket %d", objName, x->x_hostname, x->x_port, x->x_fd);
        return;
    }
    /* we get hostname and port and pass them on
       to the child thread that establishes the connection */
    x->x_hostname = hostname->s_name;
    x->x_port = fportno;
    /* start child thread */
    if(pthread_create(&x->x_threadid, &x->x_threadattr, tcpclient_child_connect, x) < 0)
        post("%s: could not create new thread", objName);
}

static void tcpclient_disconnect(t_tcpOpenVibe_tilde *x)
{
    int i;

    if (x->x_fd >= 0)
    {
        for (i = 0; i < MAX_TCPCLIENT_THREADS;++i)
        { /* wait for any sender threads to finish */
            while (x->x_tsp[i].threadisvalid != 0);
        }
        sys_closesocket(x->x_fd);
        x->x_fd = -1;
        x->x_connectstate = 0;
        x->x_ovh_read=0;
        outlet_float(x->x_connectout, 0);
        if (x->x_verbosity) post("%s: disconnected", objName);
    }
    else post("%s: not connected", objName);
}

static void tcpclient_send(t_tcpOpenVibe_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
#define BYTE_BUF_LEN 65536 // arbitrary maximum similar to max IP packet size
    static char     byte_buf[BYTE_BUF_LEN];
    int             i, j, d;
    unsigned char   c;
    float           f, e;
    size_t          sent = 0;
    char            fpath[FILENAME_MAX];
    FILE            *fptr;

#ifdef _DEBUG
    post("s: %s", s->s_name);
    post("argc: %d", argc);
#endif

    for (i = j = 0; i < argc; ++i)
    {
        if (argv[i].a_type == A_FLOAT)
        {
            f = argv[i].a_w.w_float;
            d = (int)f;
            e = f - d;
            if (e != 0)
            {
                pd_error(x, "%s_send: item %d (%f) is not an integer", objName, i, f);
                return;
            }
            if ((d < 0) || (d > 255))
            {
                pd_error(x, "%s: item %d (%f) is not between 0 and 255", objName, i, f);
                return;
            }
            c = (unsigned char)d;
            byte_buf[j++] = c;
            if (j >= BYTE_BUF_LEN)
            {
                sent += tcpclient_send_buf(x, byte_buf, j);
                j = 0;
            }
        }
        else if (argv[i].a_type == A_SYMBOL)
        {
            atom_string(&argv[i], fpath, FILENAME_MAX);
            fptr = sys_fopen(fpath, "rb");
            if (fptr == NULL)
            {
                post("%s_send: unable to open \"%s\"", objName, fpath);
                return;
            }
            rewind(fptr);
            while ((d = fgetc(fptr)) != EOF)
            {
                c = (char)(d & 0x0FF);
                byte_buf[j++] = c;
                if (j >= BYTE_BUF_LEN)
                {
                    sent += tcpclient_send_buf(x, byte_buf, j);
                    j = 0;
                }
            }
            sys_fclose(fptr);
            fptr = NULL;
            if (x->x_verbosity) post("%s_send: read \"%s\" length %d byte%s", objName, fpath, j, ((d==1)?"":"s"));
        }
        else
        {
            pd_error(x, "%s_send: item %d is not a float or a file name", objName, i);
            return;
        }
    }
    if (j > 0)
    sent += tcpclient_send_buf(x, byte_buf, j);
}


static int tcpclient_send_buf(t_tcpOpenVibe_tilde *x, char *buf, int buf_len)
{
    t_tcpclient_sender_params   *tsp = &x->x_tsp[x->x_nextthread];
    int                         i, max;

    if (x->x_blocked) return 0;
    if (x->x_fd < 0)
    {
        pd_error(x, "%s: not connected", objName);
        x->x_blocked++;
        return 0;
    }
    max = (buf_len > MAX_TCPCLIENT_SEND_BUF)? MAX_TCPCLIENT_SEND_BUF: buf_len;
    while(0 != tsp->threadisvalid); /* wait for thread to clear */
    for (i = 0; i < max; ++i)
    {
        tsp->x_sendbuf[i] = buf[i];
    }
    tsp->x_buf_len = i;
    x->x_sendbuf_len += i;
    tsp->x_x = x;
    tsp->threadisvalid = 1;
    if((tsp->x_sendresult = pthread_create(&tsp->sendthreadid, &x->x_sendthreadattr, tcpclient_child_send, tsp)) < 0)
    {
        tsp->threadisvalid = 0;
        post("%s_send_buf: could not create new thread (%d)", objName);
        clock_delay(x->x_sendclock, 0); // calls tcpclient_sent
        return 0;
    }
    x->x_nextthread++;
    if (x->x_nextthread >= MAX_TCPCLIENT_THREADS) x->x_nextthread = 0;
    return max;
}

/* tcpclient_child_send runs in sendthread */
static void *tcpclient_child_send(void *w)
{
    t_tcpclient_sender_params *tsp = (t_tcpclient_sender_params*) w;

    if (tsp->x_x->x_fd >= 0) 
    {
        tsp->x_sendresult = send(tsp->x_x->x_fd, tsp->x_sendbuf, tsp->x_buf_len, 0);
        clock_delay(tsp->x_x->x_sendclock, 0); // calls tcpclient_sent when it's safe to do so
    }
    tsp->threadisvalid = 0; /* this thread is over */
    return(tsp);
}

static void tcpclient_sent(t_tcpOpenVibe_tilde *x)
{
    t_atom  output_atom;

    if (x->x_sendresult < 0)
    {
        sys_sockerror("tcpclient: send");
        post("%s_sent: could not send data ", objName);
        x->x_blocked++;
        SETFLOAT(&output_atom, x->x_sendresult);
        outlet_anything( x->x_statusout, gensym("blocked"), 1, &output_atom);
    }
    else if (x->x_sendresult == 0)
    { /* assume the message is queued and will be sent real soon now */
        SETFLOAT(&output_atom, x->x_sendbuf_len);
        outlet_anything( x->x_statusout, gensym("sent"), 1, &output_atom);
        x->x_sendbuf_len = 0; /* we might be called only once for multiple calls to  tcpclient_send_buf */
    }
    else
    {
        SETFLOAT(&output_atom, x->x_sendresult);
        outlet_anything( x->x_statusout, gensym("sent"), 1, &output_atom);
    }
}

/* Return the send buffer size of socket, also output it on status outlet */
static int tcpclient_get_socket_send_buf_size(t_tcpOpenVibe_tilde *x)
{
    int                 optVal = 0;
    socklen_t           optLen = sizeof(int);
    t_atom              output_atom;
#ifdef _WIN32
    if (getsockopt(x->x_fd, SOL_SOCKET, SO_SNDBUF, (char*)&optVal, &optLen) == SOCKET_ERROR)
        post("%_get_socket_send_buf_size: getsockopt returned %d\n", objName, WSAGetLastError());
#else
    if (getsockopt(x->x_fd, SOL_SOCKET, SO_SNDBUF, (char*)&optVal, &optLen) == -1)
        post("%_get_socket_send_buf_size: getsockopt returned %d\n", objName, errno);
#endif
    SETFLOAT(&output_atom, optVal);
    outlet_anything( x->x_statusout, gensym("buf"), 1, &output_atom);
    return  optVal;
}

/* Set the send buffer size of socket, returns actual size */
static int tcpclient_set_socket_send_buf_size(t_tcpOpenVibe_tilde *x, int size)
{
    int optVal = size;
    int optLen = sizeof(int);
#ifdef _WIN32
    if (setsockopt(x->x_fd, SOL_SOCKET, SO_SNDBUF, (char*)&optVal, optLen) == SOCKET_ERROR)
    {
        post("%s_set_socket_send_buf_size: setsockopt returned %d\n", objName, WSAGetLastError());
#else
    if (setsockopt(x->x_fd, SOL_SOCKET, SO_SNDBUF, (char*)&optVal, optLen) == -1)
    {
        post("%s_set_socket_send_buf_size: setsockopt returned %d\n", objName, errno);
#endif
        return 0;
    }
    else return (tcpclient_get_socket_send_buf_size(x));
}

/* Get/set the send buffer size of client socket */
static void tcpclient_buf_size(t_tcpOpenVibe_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    float   buf_size = 0;

    if(x->x_connectstate == 0)
    {
        post("%s_buf_size: no clients connected", objName);
        return;
    }
    /* get size of buffer (first element in list) */
    if (argc > 0)
    {
        if (argv[0].a_type != A_FLOAT)
        {
            post("%s_buf_size: specify buffer size with a float", objName);
            return;
        }
        buf_size = atom_getfloatarg(0, argc, argv);
        x->x_fdbuf = tcpclient_set_socket_send_buf_size(x, (int)buf_size);
        if (x->x_verbosity) post("%s_buf_size: set to %d", objName, x->x_fdbuf);
        return;
    }
    x->x_fdbuf = tcpclient_get_socket_send_buf_size(x);
    return;
}

static void tcpclient_rcv(t_tcpOpenVibe_tilde *x)
{
    int             sockfd = x->x_fd;
    int             ret; //bytes received
    int             i;
    fd_set          readset;
    fd_set          exceptset;
    struct timeval  ztout;
    // OpenVibe Stuff
    t_OpenVibe_header ovh;
    int             bytes_processed=0;// because maybe we need to read the openvibe header, we need a starting point for the stream



    if(x->x_connectstate)
    {
        /* check if we can read/write from/to the socket */
        FD_ZERO(&readset);
        FD_ZERO(&exceptset);
        FD_SET(x->x_fd, &readset );
        FD_SET(x->x_fd, &exceptset );

        ztout.tv_sec = 0;
        ztout.tv_usec = 0;

        ret = select(sockfd+1, &readset, NULL, &exceptset, &ztout);
        if(ret < 0)
        {
            pd_error(x, "%s: unable to read from socket", objName);
            sys_closesocket(sockfd);
            return;
        }
        if(FD_ISSET(sockfd, &readset) || FD_ISSET(sockfd, &exceptset))
        {
            /* read from server */
            ret = recv(sockfd, x->x_msginbuf, MAX_TCPCLIENT_SEND_BUF, 0);
            if(ret > 0)
            {
#ifdef _DEBUG
                x->x_msginbuf[ret] = 0;
                if (x->x_verbosity ) post("%s: received %d bytes ", objName, ret);
#endif
                if (x->x_dump)tcp_client_hexdump(x, ret);
                /* find sender's ip address and output it */
                x->x_addrbytes[0].a_w.w_float = (x->x_addr & 0xFF000000)>>24;
                x->x_addrbytes[1].a_w.w_float = (x->x_addr & 0x0FF0000)>>16;
                x->x_addrbytes[2].a_w.w_float = (x->x_addr & 0x0FF00)>>8;
                x->x_addrbytes[3].a_w.w_float = (x->x_addr & 0x0FF);
                outlet_list(x->x_addrout, &s_list, 4L, x->x_addrbytes);

                if (!(x->x_ovh_read) && (ret>=SIZE_OPENVIBE_HEADER))
                {
                    bytes_processed+=SIZE_OPENVIBE_HEADER;
                    ovh=OpenVibe_header_from_bytes(x->x_msginbuf);
                    post_OpenVibeHeader(&ovh);
                    x->x_ovh_read=1; // we dont need to read and skip this until next conection
                }
                
                /*
                 Open Vibe header is not needed in the outlet stream so we skip it   
                 moreover, since OpenVibe samples are double instead of byte or float
                 we need to get the floats from doubles for chunks of 8 bytes. 
                 Loss of precision must be evaluated carefully. 
                */
                EnterCriticalSection(&cs);
                x->x_msgout_from=x->x_msgout_to=0; // pending a pipe implemetation in order 
                                                   // to assure no packet is lost
                x->x_msgout_to=-1;
                for (i = bytes_processed; i < ret-SIZE_OPENVIBE_SAMPLE; i+=SIZE_OPENVIBE_SAMPLE)
                {   
                    x->x_msgout_to++;
                    x->x_msgoutbuf[x->x_msgout_to]= (t_sample)bytes2double(&(x->x_msginbuf[i]));
                    //x->x_msgoutbuf[x->x_msgout_to]=sample_fake_triangular();
                    // RMBR: this is the point of conversion from bytes to double and from double to float.
                      
                }
                LeaveCriticalSection(&cs);
               
            }
            else
            {
                if (ret < 0)
                {
                    sys_sockerror("tcpclient: recv");
                    tcpclient_disconnect(x);
                }
                else
                {
                    if (x->x_verbosity) post("%s: connection closed for socket %d\n", objName, sockfd);
                    tcpclient_disconnect(x);
                }
            }
        }
    }
    else post("%s: not connected", objName);
}

static void tcpclient_poll(t_tcpOpenVibe_tilde *x)
{
    if(x->x_connectstate)
        tcpclient_rcv(x);	/* try to read in case we're connected */
    clock_delay(x->x_poll, DEFPOLLTIME);	/* see you later */
}

static void tcpclient_unblock(t_tcpOpenVibe_tilde *x)
{
    x->x_blocked = 0;
}




//// FIN DE ZONA COPIADA DE TCPCLIENT

/**
 * this is the "constructor" of the class
 * this method is called whenever a new object of this class is created
 * the name of this function is arbitrary and is registered to Pd in the 
 * tcpOpenVibe_tilde_setup() routine
 */



__declspec(dllexport) void *tcpOpenVibe_tilde_new(t_symbol *s, int argc, t_atom *argv) //ahora, numero variable de argumentos
{
  int i;
  /*
   * call the "constructor" of the parent-class
   * this will reserve enough memory to hold "t_tcpOpenVibe_tilde"
   */
  t_tcpOpenVibe_tilde *x = (t_tcpOpenVibe_tilde *)pd_new(tcpOpenVibe_tilde_class);
  // ZONA DE TCP

  
    x->x_signalout = outlet_new(&x->x_obj, &s_signal); /*create a signal-outlet for received data */   
    x->x_addrout = outlet_new(&x->x_obj, &s_list);
    x->x_connectout = outlet_new(&x->x_obj, &s_float);	/* connection state */
    x->x_statusout = outlet_new(&x->x_obj, &s_anything);/* last outlet for everything else */
    x->x_sendclock = clock_new(x, (t_method)tcpclient_sent);
    x->x_clock = clock_new(x, (t_method)tcpclient_tick);
    x->x_poll = clock_new(x, (t_method)tcpclient_poll);
    x->x_verbosity = 1; /* default post status changes to main window */
    x->x_fd = -1;
    x->x_msgout_from=0;
    x->x_msgout_to=0;
    
    /* convert the bytes in the buffer to floats in a list */

    for (i = 0; i < MAX_TCPCLIENT_SEND_BUF; ++i)
    {
        //x->x_msgoutbuf[i].a_type = A_FLOAT;
        //x->x_msgoutbuf[i].a_w.w_float = 0;
        x->x_msgoutbuf[i]=0.0; // todo cleaning
    }
    for (i = 0; i < 4; ++i)
    {
        x->x_addrbytes[i].a_type = A_FLOAT;
        x->x_addrbytes[i].a_w.w_float = 0;
    }
    x->x_addr = 0L;
    x->x_blocked = 1;
    x->x_connectstate = 0;
    x->x_ovh_read=0;
    x->x_nextthread = 0;
    /* prepare child threads */
    if(pthread_attr_init(&x->x_threadattr) < 0)
        post("%s: warning: could not prepare child thread", objName);
    if(pthread_attr_setdetachstate(&x->x_threadattr, PTHREAD_CREATE_DETACHED) < 0)
        post("%s: warning: could not prepare child thread", objName);
    if(pthread_attr_init(&x->x_sendthreadattr) < 0)
        post("%s: warning: could not prepare child thread", objName);
    if(pthread_attr_setdetachstate(&x->x_sendthreadattr, PTHREAD_CREATE_DETACHED) < 0)
        post("%s: warning: could not prepare child thread", objName);
    clock_delay(x->x_poll, 0);	/* start polling the input */
   /*
   * return the pointer to the class - this is mandatory
   * if you return "0", then the object-creation will fail
   */

  InitializeCriticalSection(&cs);
  return (void *)x;
}


/**
 * define the function-space of the class
 * within a single-object external the name of this function is special
 */

__declspec(dllexport) void tcpOpenVibe_tilde_setup(void)
{
   char    aboutStr[MAXPDSTRING];

    snprintf(aboutStr, MAXPDSTRING, "%s: (GPL)\n Avidabits - Rosa Bernárdez, based on Martin Peach tcpclient, compiled for pd-%d.%d on %s %s", objName, PD_MAJOR_VERSION, PD_MINOR_VERSION, __DATE__, __TIME__);

#if PD_MAJOR_VERSION==0 && PD_MINOR_VERSION<43
    post(aboutStr);
#else
    logpost(NULL, 3, aboutStr);
#endif

  /* create a new class */
  tcpOpenVibe_tilde_class = class_new(gensym(objName), /* the object's name is "tcpOpenVibe~" */
			       (t_newmethod)tcpOpenVibe_tilde_new, /* the object's constructor" */
			       (t_method)tcpOpenVibe_tilde_free,   /*  destructor */
			       sizeof(t_tcpOpenVibe_tilde),        /* the size of the data-space */
			       CLASS_DEFAULT,                   /* a normal pd object */
                   A_GIMME,                         /* an arbitrary number of arguments */
                   0);
  
  /* set the name of the help-patch to "help-tcpOpenVibe~"(.pd) */
  class_sethelpsymbol(tcpOpenVibe_tilde_class, gensym("help-tcpOpenVibe~"));

  /* attach functions to messages */
    class_addmethod(tcpOpenVibe_tilde_class, (t_method)tcpclient_connect, gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(tcpOpenVibe_tilde_class, (t_method)tcpclient_disconnect, gensym("disconnect"), 0);
    class_addmethod(tcpOpenVibe_tilde_class, (t_method)tcpclient_send, gensym("send"), A_GIMME, 0);
    class_addmethod(tcpOpenVibe_tilde_class, (t_method)tcpclient_buf_size, gensym("buf"), A_GIMME, 0);
    class_addmethod(tcpOpenVibe_tilde_class, (t_method)tcpclient_unblock, gensym("unblock"), 0);
    class_addmethod(tcpOpenVibe_tilde_class, (t_method)tcpclient_verbosity, gensym("verbosity"), A_FLOAT, 0);
    class_addmethod(tcpOpenVibe_tilde_class, (t_method)tcpclient_dump, gensym("dump"), A_FLOAT, 0);
    class_addlist(tcpOpenVibe_tilde_class, (t_method)tcpclient_send);

  /// registering the dsp performing function.
  post("tcpOpenVibe_tilde_setup:  class_addmethod tcpOpenVibe_tilde_dsp");
  class_addmethod(tcpOpenVibe_tilde_class,
        (t_method)tcpOpenVibe_tilde_dsp, gensym("dsp"), 0);

}

/// free, 
__declspec(dllexport)  void tcpOpenVibe_tilde_free(t_tcpOpenVibe_tilde *x)
{
    if (x->x_verbosity) post("tcpOpenVibe_tilde_free...");
    tcpclient_disconnect(x);
    clock_free(x->x_poll);
    clock_free(x->x_clock);
    
    /* free any resources associated with the given outlet */
    outlet_free(x->x_signalout);
    outlet_free(x->x_addrout);
    outlet_free(x->x_connectout);
    outlet_free(x->x_statusout);

    DeleteCriticalSection(&cs);
    if (x->x_verbosity) post("...tcpOpenVibe_tilde_free");
}


//////////////////////////////////////////////////////////////////
/* 
  OPENVIBE RELATED CODE
*/

double bytes2double(unsigned char *c)
{
    int i=0; //just a counter... 
    //ATENCION: sólo válido entre sistemas little-endian
    union {
        double dconverted;
        unsigned char c[sizeof(double)];
    } united_double;

    for (i=0; i<sizeof(double); i++)
    {   
        united_double.c[i]=c[i];
    }
    return united_double.dconverted; 
}

/* Get OpenVibe header as documented at
  http://openvibe.inria.fr//documentation/2.0.1/Doc_BoxAlgorithm_TCPWriter.html

| Name                | Type          | Bytes from start |
| ------------------- | ------------- | ---------------- |
| Format version      | uint32        | 0                |
| Endianness          | uint32        | 4  (network byte order, 0==unknown, 1==little, 2==big, 3==pdp)
| Sampling frequency  | uint32        | 8                |
| Number of channels  | uint32        | 12               |
| Samples per chunk   | uint32        | 16               |
| Reserved0           | uint32        | 20               |
| Reserved1           | uint32        | 24               |
| Reserved2           | uint32        | 28               |
*/

t_OpenVibe_header OpenVibe_header_from_bytes(char *bytes)
{ 
  t_OpenVibe_header ovh;
  memcpy(&ovh.format_version, &(bytes[0]), sizeof(ovh.format_version));
  memcpy(&ovh.endianess, &(bytes[4]), sizeof(ovh.endianess));
  memcpy(&ovh.sampling_frecuency, &(bytes[8]), sizeof(ovh.sampling_frecuency));
  memcpy(&ovh.n_channels, &(bytes[12]), sizeof(ovh.n_channels));
  memcpy(&ovh.samples_per_chunk, &(bytes[16]), sizeof(ovh.samples_per_chunk));
  memcpy(&ovh.reserved0, &(bytes[20]), sizeof(ovh.reserved0));
  memcpy(&ovh.reserved1, &(bytes[24]), sizeof(ovh.reserved1));
  memcpy(&ovh.reserved2, &(bytes[28]), sizeof(ovh.reserved2));
  return ovh;
}

void post_OpenVibeHeader(t_OpenVibe_header *ovh)
{
  post("OpenVibe Header received:");
  post("ovh.format_version: %d", ovh->format_version);
  post("ovh.endianess: %d", ovh->endianess);
  post("ovh.sampling_frecuency: %d", ovh->sampling_frecuency);
  post("ovh.n_channels: %d", ovh->n_channels);
  post("ovh.samples_per_chunk: %d", ovh->samples_per_chunk);
  post("ovh.reserved0: %d", ovh->reserved0);
  post("ovh.reserved1: %d", ovh->reserved1);
  post("ovh.reserved2: %d", ovh->reserved2);
  if (ovh->n_channels >1) post("YOUR ATTENTION: this version is for one channel only");
}


// NEW FUNCS NEED IN ORDER TO CREATE THE TILDE OUTPUT


t_int *tcpOpenVibe_tilde_perform(t_int *w)
{
  /* the first element is a pointer to the dataspace of this object */
  t_tcpOpenVibe_tilde *x =(t_tcpOpenVibe_tilde *)(w[1]);
  /* here comes the signalblock that will hold the output signal */
  t_sample  *out =    (t_sample *)(w[2]);
  /* all signalblocks are of the same length */
  int          n =           (int)(w[3]);
  /* just a counter */
  int i=0;
  int samples2read=0;

  EnterCriticalSection(&cs);
  samples2read=1+x->x_msgout_to-x->x_msgout_from;
  
  for(i=0; i<min(n, samples2read); i++)
  {   
      //out[i]=sample_fake_triangular();
      out[i]=x->x_msgoutbuf[i+x->x_msgout_to];//.a_w.w_float;
      x->x_msgout_from++;
  }
  LeaveCriticalSection(&cs);

  /* return a pointer to the dataspace for the next dsp-object */
  return (w+4);
}

void tcpOpenVibe_tilde_dsp(t_tcpOpenVibe_tilde *x, t_signal **sp)
{
  /* add tcpOpenVibe_tilde_perform() to the DSP-tree;
   * the tcpOpenVibe_tilde_perform() will expect "3" arguments (packed into an
   * t_int-array), which are:
   * the objects data-space, 1 signal vectors (which happen to be output signal)
   * and the length of the signal vector (all vectors are of the same length)
   */
   if (x->x_verbosity)
      post("tcpOpenVibe_tilde_dsp:dsp_add tcpOpenVibe_tilde_perforn\n");
  dsp_add(tcpOpenVibe_tilde_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}
