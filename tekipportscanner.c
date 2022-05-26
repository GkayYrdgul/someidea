#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>




char char_ip_adress [50];
int dongu_sayaci=1;
int islem_sayaci=0;
int son_threadin_sayisi;
pthread_t thread_id[1000]; //thread dizisi
int bilindik_port_listesi [] = {80,443,21,22,3389,53,9000,57000,1194,465,993};
int max_port_range=65535;
int max_port_list[65535];
int kalan_port_sayisi;

int max_thread_sayisi=1000; //program calisirken kullanilacak thread sayisi 1000 den fazla yapacaksaniz thread dizisini arttirmalisiniz

void *thread_port_taramasi(void *data);

int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        printf("./portscan [ip_adres] [option] \n");
        exit(1);
    }

    strcpy(char_ip_adress,argv[1]);

    /*Kullanicidan ip ve islem icin veri girmesi isteniyor islem turunu sectigi veri arv[2]. Asagidaki if yapisi eger 1 girildiyse 65000 portuda tariyor eger
    2 girildiyse bilindik portlar listesindeki portlari tariyor. Dongu yapisinin icinde ise kalan port sayisina, eger 1 secildiyse butun portlarin sayisi eger
    2 secildiyse, port listesinin eleman sayisi veriliyor bu threadler islem yaptikca yapilan islem kadar cikarilip bitene kadar azaltilacak, bittiginde program
    sonlanacak. Threadler olusturulup verilen portu taradiktan sonra kapaniyor sonrasinda yenileri olusuyor ve olusan yeni threadlere ayni id ler ataniyor.
    islem sayaci ve dongu sayaci isminde 2 tane sayac var bunlar her thread olustugunda artiyor yani her port tarandiginda artiyor. Islem sayaci islemin sonunda
    kalan port sayisindan cikariliyor ve her islem yapildiginde bunu yaptiktan sonra sifirlaniyor bu sayede taranan portlar kalan portlardan cikarilarak geriye
    kac port kaldigi goruluyor. Dongu sayaci ise asla sifirlanmiyor threadlere portlarin bulundugu listedeki indexleri veriyor.*/


    if(1==atoi(argv[2]))
    {
        kalan_port_sayisi=max_port_range;
        for(int i=1; i<=65535; i++)
        {
            max_port_list[i]=i;
        }
        while(0!=kalan_port_sayisi)
        {
            if(kalan_port_sayisi>=max_thread_sayisi)
            {
                while(islem_sayaci<max_thread_sayisi)
                {
                    pthread_create(&thread_id[islem_sayaci],NULL,thread_port_taramasi,&max_port_list[dongu_sayaci]);
                    islem_sayaci++;
                    dongu_sayaci++;
                }
                pthread_join(thread_id[islem_sayaci-1],NULL);
                kalan_port_sayisi-=islem_sayaci;
                islem_sayaci=0;
                printf("kalan port sayisi %d\n ",kalan_port_sayisi);
            }
            else
            {
                while(islem_sayaci<kalan_port_sayisi)
                {
                    pthread_create(&thread_id[islem_sayaci],NULL,thread_port_taramasi,&max_port_list[dongu_sayaci]);
                    islem_sayaci++;
                    dongu_sayaci++;
                }
                pthread_join(thread_id[islem_sayaci-1],NULL);
                kalan_port_sayisi-=islem_sayaci;
                printf("kalan port sayisi %d\n",kalan_port_sayisi);
            }
        }
    }
    else if(2==atoi(argv[2]))
    {
        kalan_port_sayisi=sizeof(bilindik_port_listesi)/sizeof(int);
        while(0!=kalan_port_sayisi)
        {
            while(islem_sayaci<kalan_port_sayisi)
            {
                pthread_create(&thread_id[islem_sayaci],NULL,thread_port_taramasi,&bilindik_port_listesi[dongu_sayaci-1]);
                islem_sayaci++;
                dongu_sayaci++;
            }
            pthread_join(thread_id[islem_sayaci-1],NULL);
            kalan_port_sayisi-=islem_sayaci;
            printf("kalan port sayisi %d\n",kalan_port_sayisi);
        }
    }
}





void *thread_port_taramasi(void *data)
{
    unsigned int port_numarasi;
    int *port_pointer=(int *)data;

    port_numarasi=htons(*port_pointer);

    struct sockaddr_in client_addr;
    struct timeval timeout_struct;
    int socket_file_D, socket_baglanti_durumu;
    char char_ip_adresi[50];
    int ip_octet_1=0,ip_octet_2=0,ip_octet_3=0,ip_octet_4=0;
    int socket_flag;
    unsigned long long_ip_adress;

    timeout_struct.tv_sec=2;
    timeout_struct.tv_usec=0;

    client_addr.sin_port=port_numarasi;
    client_addr.sin_family=AF_INET;

    sscanf(char_ip_adress,"%d.%d.%d.%d",&ip_octet_1,&ip_octet_2,&ip_octet_3,&ip_octet_4);
    sprintf(char_ip_adresi,"%d.%d.%d.%d",ip_octet_4,ip_octet_3,ip_octet_2,ip_octet_1);
    long_ip_adress=inet_addr(char_ip_adresi);

    client_addr.sin_addr.s_addr=htonl(long_ip_adress);

    sscanf(char_ip_adresi,"%d.%d.%d.%d",&ip_octet_1,&ip_octet_2,&ip_octet_3,&ip_octet_4);
    sprintf(char_ip_adresi,"%d.%d.%d.%d",ip_octet_4,ip_octet_3,ip_octet_2,ip_octet_1);


    socket_file_D=socket(AF_INET,SOCK_STREAM,0);

    if ((socket_flag=fcntl(socket_file_D,F_GETFL,NULL))<0)
    {
        exit(1);
        printf("Get Socket Flag error");
    };
    if (fcntl(socket_file_D,F_SETFL,socket_flag |O_NONBLOCK)<0)
    {
        exit(1);
        printf("SET Socket Flag error");
    }

    if((socket_baglanti_durumu=connect(socket_file_D,(struct sockaddr *)&client_addr,sizeof(client_addr)))<0)
    {
        if(errno==EINPROGRESS)
        {
            fd_set socket_set_ayari;
            FD_ZERO(&socket_set_ayari);
            FD_SET(socket_file_D,&socket_set_ayari);
            socket_baglanti_durumu=select(socket_file_D+1,NULL,&socket_set_ayari,NULL,(struct timeval *)&timeout_struct);
        }
    }
    else
    {
        socket_baglanti_durumu=1;
    }
    if(fcntl(socket_file_D,F_SETFL,socket_flag)<0)
    {
        exit(1);
        printf("Reset Socket Flag error");
    }
    if(thread_id[0]==pthread_self())
    {
        pthread_join(thread_id[0],NULL);
    }
    else
    {
        for(int i=0; i<max_thread_sayisi; i++)
        {
            if(thread_id[i]==pthread_self())
            {
                pthread_join(thread_id[i-1],NULL);
            }
        }
    }
    if(socket_baglanti_durumu==1)
    {
        printf("\nip: %s  long ip: %lu\n",char_ip_adresi,long_ip_adress);
        printf("Port:%d \n",ntohs(port_numarasi));
        printf("Soket Durumu: %d \n",socket_file_D);
        printf("Aktif \n\n");
    }

/*  else
    {
        printf("ip: %s  long ip: %lu\n",char_ip_adresi,long_ip_adress);
        printf("Port:%d \n",ntohs(port_numarasi));
        printf("Soket Durumu: %d \n",socket_file_D);
        printf("Kapali \n\n");
    }
*/
    close(socket_file_D);
    return NULL;
}
