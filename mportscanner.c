#include <math.h>
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

char char_ip_adresi[50]; //Girilen ip adresi
int subnet_mask_degeri; //Girilen Subnet Mask deðeri
int ip_octet_1=0,ip_octet_2=0,ip_octet_3=0,ip_octet_4=0; //Ýp deðerinin octetler halindeki decimal degeri
int submask_octet_1=0,submask_octet_2=0,submask_octet_3=0,submask_octet_4=0; //subnet mask degerinin octetler halindeki decimal degeri
int binary_ip_adresi[4][8]; //ip adresinin binary hali
int binary_subnet_mask[4][8]; // Subnet mask degerinin binary hali
int host_baslangic_adresi=0, host_bitis_adresi=0; //baslangic adresi network adresinin 4. octetinin 1 fazlasi bitis adresi ise Broadcast adresinin 1 eksigi
int host_sayisi=0; //Normal halindeki host sayisi
int submask24_network_sayisi=0; //24 subnetlere bolundugunde ki host sayisi
char char_host_baslangic_adresi [50];


void intager_binary_donusumu(int verilen_int_deger,int verilen_int_dizi[8]); //intager degeri binary degerine donusturen fonksiyon
void subnet_mask_binary_hesaplama();                             //verilen subnet degerini binary seklinde listeye atayan fonksiyon
void andleme_islemi(int ip_octet[8], int sub_mask_octet[8]);     //binary degerleri arasinda and isleminin yapildigi fonksiyon
void binary_degil_islemi(int verilen_octet[8]);                  //binary degerleri tersine ceviren Not yani degil fonksiyonu
int binary_intager_donusumu(int atanacak_octet, int verilen_binary_dizi[8]); //binary diziden intager sayiya donusturen fonksiyon

/*YUKARIDAKI KISIM SUBNET PROGRAMINDA KULLANILAN DEGISKENLER VE FONKSIYONLAR ASAGIDAKI ISE PORT TARAMASINI YAPAN PROGRAMDA KULLANDIKLARIM*/

pthread_barrier_t thread_barieri; //threadleri bekleten barrier fonksiyonu icin barrierin id si

char liste_icerigi[4200000][50]; //tum iplerin tutuldugu liste
int dongu_sayaci=0; //dongude ip listesindeki indexlerde gezinmek icin olusturdugum sayac
int liste_sayaci=0; //listede tutulan ip sayisi
int islem_sayaci=0; //100 threadlik bir islemin sayaci her 100 thread calistiginda 1 artiyor
unsigned int port_numarasi=0; //girilen port numarasi
int son_threadin_sayisi; //islemlerden sonra olusan son threadin indexi
pthread_t thread_id[4200000]; //threadler kendilerini yok ediyorlar fakat listedeki ip leri listenin indexiyle ayni threadin indexine yolladigim icin ayni boyutta thread id dizisi


void *thread_port_taramasi(void *data); //port taramasinin yapildigi fonksiyon


int main (int argc, char *argv[])
{
    if(argc!=4)
    {
        printf("./mportscanner [ip] [mask] [taranacak_port]\n");
        exit(0);
    }
    port_numarasi=htons(atoi(argv[3]));
    strcpy(char_ip_adresi,argv[1]);
    sscanf(char_ip_adresi,"%d.%d.%d.%d",&ip_octet_1,&ip_octet_2,&ip_octet_3,&ip_octet_4);
    subnet_mask_degeri=atoi(argv[2]);

    if(subnet_mask_degeri<10||subnet_mask_degeri>24)
    {
        perror("Hatali Deger: subnet degeri 9 dan yuksek 25 den az olmali \n ");
        exit(1);
    }
    if(ip_octet_1<=0||ip_octet_1>255)
    {
        perror("Hatali Deger: yanlis ip degeri");
        exit(2);
    }

    printf("\n \nGirilen Ip Adresi:  %s /%d \n",char_ip_adresi,subnet_mask_degeri);

    host_sayisi=pow(2,32-subnet_mask_degeri)-2;            //host sayisi yani subnet mask degerindeki 0 sayisinin 2 nin ustu seklinde alarak toplam host sayisini buldum -2 network broadcast adresleri

    submask24_network_sayisi=pow(2,32-subnet_mask_degeri-8); //toplam host sayisinden son octetin cikarilmis hali 24 luk subnetlere ayrilinceki host sayisi

    intager_binary_donusumu(ip_octet_1,binary_ip_adresi[0]);  //girilen ip adresinin binaryye donusturulmesi
    intager_binary_donusumu(ip_octet_2,binary_ip_adresi[1]);
    intager_binary_donusumu(ip_octet_3,binary_ip_adresi[2]);
    intager_binary_donusumu(ip_octet_4,binary_ip_adresi[3]);

    subnet_mask_binary_hesaplama();                           //girilen subnet degerinin binary subnet mask adresine donusturulmesi

    andleme_islemi(binary_ip_adresi[0],binary_subnet_mask[0]);  //binary ip adresleririn binary subnet mask adresleriyle octet octet and islemi yapilmasi
    andleme_islemi(binary_ip_adresi[1],binary_subnet_mask[1]);
    andleme_islemi(binary_ip_adresi[2],binary_subnet_mask[2]);
    andleme_islemi(binary_ip_adresi[3],binary_subnet_mask[3]);


    ip_octet_1=binary_intager_donusumu(ip_octet_1,binary_ip_adresi[0]);    //and isleminden sonra binary den int degerlere donusturulmesi
    ip_octet_2=binary_intager_donusumu(ip_octet_2,binary_ip_adresi[1]);
    ip_octet_3=binary_intager_donusumu(ip_octet_3,binary_ip_adresi[2]);
    ip_octet_4=binary_intager_donusumu(ip_octet_4,binary_ip_adresi[3]);

    host_baslangic_adresi+=1+ip_octet_4;                                    //network adresi bulundugu icin son octetin 1 fazlasini alarak host baslangic adresini buldum

    submask_octet_1=binary_intager_donusumu(submask_octet_1,binary_subnet_mask[0]);   //subnet mask adresinin int donusumu
    submask_octet_2=binary_intager_donusumu(submask_octet_2,binary_subnet_mask[1]);
    submask_octet_3=binary_intager_donusumu(submask_octet_3,binary_subnet_mask[2]);
    submask_octet_4=binary_intager_donusumu(submask_octet_4,binary_subnet_mask[3]);

    printf("\nSubnet Mask: %d.%d.%d.%d \n",submask_octet_1,submask_octet_2,submask_octet_3,submask_octet_4);

    printf("\nNetwork Adresi: %d.%d.%d.%d \n",ip_octet_1,ip_octet_2,ip_octet_3,ip_octet_4);

    printf("\nHost Baslangic Adresi: %d.%d.%d.%d \n",ip_octet_1,ip_octet_2,ip_octet_3,host_baslangic_adresi);

    sprintf(char_host_baslangic_adresi,"%d.%d.%d.%d\n",ip_octet_1,ip_octet_2,ip_octet_3,host_baslangic_adresi); //host baslangic adresini kaydettim

    binary_degil_islemi(binary_subnet_mask[0]);    //subnet mask adresinin binary degilini aliyorum cunku Broadcast adresi network adresiyle subnet mask adresinin degilinin toplanmasiyla bulunuyor
    binary_degil_islemi(binary_subnet_mask[1]);
    binary_degil_islemi(binary_subnet_mask[2]);   //yani 1 ler 0 olup 0 lar 1 oldugu icin diger networke kalan degeri bulmus oluyoruz
    binary_degil_islemi(binary_subnet_mask[3]);

    submask_octet_1=binary_intager_donusumu(submask_octet_1,binary_subnet_mask[0]);   //degili alinan subnet masklarin tekrar intager donusumu
    submask_octet_2=binary_intager_donusumu(submask_octet_2,binary_subnet_mask[1]);
    submask_octet_3=binary_intager_donusumu(submask_octet_3,binary_subnet_mask[2]);
    submask_octet_4=binary_intager_donusumu(submask_octet_4,binary_subnet_mask[3]);

    ip_octet_1+=submask_octet_1;
    ip_octet_2+=submask_octet_2;     //subnet mask degilinin network adresiyle toplanarak Broadcast adresinin bulunmasi
    ip_octet_3+=submask_octet_3;
    ip_octet_4+=submask_octet_4;

    host_bitis_adresi=ip_octet_4-1;
       //Broadcast adresinin 4. octetinin 1 eksigi son host adresi
    printf("\nHost Bitis Adresi: %d.%d.%d.%d \n",ip_octet_1,ip_octet_2,ip_octet_3,host_bitis_adresi);
    printf("\nBroadcast Adresi: %d.%d.%d.%d \n \n",ip_octet_1,ip_octet_2,ip_octet_3,ip_octet_4);
    printf("Host Sayisi= %d\n \n",host_sayisi);
    printf("/24 Network Sayisi:%d \n\n",submask24_network_sayisi);

    /* Buraya kadar subnet programiniydi burdan sonra ipleri listeye atayıp portlara gönderiyorum   port scan buradan itibaren basliyor                */

    sscanf(char_host_baslangic_adresi,"%d.%d.%d.%d",&ip_octet_1,&ip_octet_2,&ip_octet_3,&ip_octet_4); //Baslangic adresininden itibaren listelemek icin baslangic adresini okuyorum

    host_sayisi+=1;  //24 luk subnette 254 host var ama biz 255 er ilerledigimiz icin 1 arttiriyorum

    liste_sayaci=host_sayisi;   //toplam host sayisi aslinde listedeki toplam ip sayisi olacak o yuzden esitler
    for(int i=0; i<=host_sayisi; i++)
    {
        sprintf(liste_icerigi[i],"%d.%d.%d.%d \n",ip_octet_1,ip_octet_2,ip_octet_3,ip_octet_4);//baslangic adresinden itibaren son host adresine kadar listeliyorum
        ip_octet_4++;
        if(ip_octet_4==256){ip_octet_4=1; ip_octet_3++;}
        if(ip_octet_3==256){ip_octet_3=0; ip_octet_2++;}
        if(ip_octet_2==256){ip_octet_2=0; ip_octet_1++;} //octetleri denetleyerek arttiriyorum ip duzenini saglamak icin

    }
/* Alt kisimda ise 100 thread calisacagi icin listeyi 100 er 100 er okuyup threadleri olusturuyorum eger listede 100 den az veri kaldiysa kalan veri kadar thread olusturuyorum

 ayrica her 100 thread olustugunda barrier fonksiyonu ve join fonksiyonu kullanarak 100 thread i ayni anda baslatip son thread bitene kadar bekliyorum sonra donguye devam ediyorum*/

    while(liste_sayaci!=0) // listedeki eleman sayisi eger 0 degilse
    {
        if(liste_sayaci>=100) //listedeki eleman sayisi eger 100 veya buyukse
        {
            dongu_sayaci=islem_sayaci*100;  //dongu sayaci listedenin indexlerini temsil ediyor
            son_threadin_sayisi=islem_sayaci*100+99;  //son threadin sayisi son threadin indexini gosteriyor ve son thread islemini bitirene kadar beklememizi sagliyor index gosterdigi icin 100 uncu sayi index ken 99 la bitecek

            while(dongu_sayaci!=islem_sayaci*100+100) //islem sayaci 100 threadlik bir islemi temsil ediyor o yuzden sayac*100 suan bakilan ip olurken +100 sonraki 100 ip olacak
            {
                pthread_create(&thread_id[dongu_sayaci],NULL,thread_port_taramasi,&liste_icerigi[dongu_sayaci]); //threadleri olusturuyorum ve hepsine listedeki ipleri olustururken sirayla atiyorum
                dongu_sayaci++;
                 // Olusan threadleri thread_port_taramasi adinda olusturdugum fonksiyona gonderirken listedeki ip leri parametre olarak yolluyorum
            }
            pthread_barrier_init(&thread_barieri,NULL,100); //burda 100 thread olusana kadar threadleri bekletiyorum 100 olunca otomatik calisacaklar
            printf("Ip sayisi=%d \n",liste_sayaci);
            pthread_join(thread_id[son_threadin_sayisi],NULL); //join fonksiyonuyla olusacak son threadin indexini vererek son threadin isi bitene kadar main fonksiyonu bekletiyorum
            pthread_barrier_destroy(&thread_barieri); //100 thread olana kadar bekleten barrieri kaldiriyorum
            liste_sayaci-=100; //100 islem bittigi icin 100 ip eksiltiyorum
            dongu_sayaci=0; //yukarida tekrar atanacagi icin sayaci 0 liyorum
            islem_sayaci++; //islem sayaci 100 threadlik bir islemi temsil ediyor ve her 100 thread olustugunda 1 artiyor islem bittigi icin arttiriyorum
        }
        else
            /* ELSE KISMI YUKARININ AYNISI SADECE LISTEDE KALAN IP SAYISI 100 DEN KUCUKKEN KALAN IP SAYISI KADAR THREAD OLUSTURUYOR */
        {
            dongu_sayaci=islem_sayaci*100;
            son_threadin_sayisi=liste_sayaci+islem_sayaci*100-2; //xx.xx.xx.0 ve xx.xx.xx.255 i cikardigim icin -2
            while(dongu_sayaci!=liste_sayaci+islem_sayaci*100-1)
            {
                pthread_create(&thread_id[dongu_sayaci],NULL,thread_port_taramasi,&liste_icerigi[dongu_sayaci]);
                dongu_sayaci++;
            }
            pthread_barrier_init(&thread_barieri,NULL,liste_sayaci-1); //100 thread olana kadar degilde listedeki ip sayisi kadar thread olana kadar bekletiyorum
            printf("Ip sayisi=%d \n",liste_sayaci);
            pthread_join(thread_id[son_threadin_sayisi],NULL);
            pthread_barrier_destroy(&thread_barieri);
            liste_sayaci=0;
            dongu_sayaci=0;
            islem_sayaci++;
        }
    }
}

void *thread_port_taramasi(void *data) //THREADLERIN CALISTIGI FONKSIYON port taramasi ve soketler burda yapiliyor
{
    char *ip_adres_pointer=(char *)data;
    struct sockaddr_in client_addr;
    struct timeval timeout_struct;
    int socket_file_D, socket_baglanti_durumu; //soket file descriptor ve soketi olusturduktan sonra denetleyecegim degisken
    char char_ip_adresi[50];
    int ip_octet_1=0,ip_octet_2=0,ip_octet_3=0,ip_octet_4=0;
    int socket_flag; //soketi nonblock yaparken kullandigim file flag degiskeni
    unsigned long long_ip_adress;

    timeout_struct.tv_sec=2; //time out suresi 2 saniye
    timeout_struct.tv_usec=0;

    client_addr.sin_port=port_numarasi; //soketin port numarasi girilen port numarasi
    client_addr.sin_family=AF_INET;  //ipv4

    sscanf(ip_adres_pointer,"%d.%d.%d.%d",&ip_octet_1,&ip_octet_2,&ip_octet_3,&ip_octet_4); //inet_addr ip adresinin tersini long adresine ceviriyor bu yuzden ip adresini ters ceviriyorum
    sprintf(char_ip_adresi,"%d.%d.%d.%d",ip_octet_4,ip_octet_3,ip_octet_2,ip_octet_1);
    long_ip_adress=inet_addr(char_ip_adresi);  //ip to long

    client_addr.sin_addr.s_addr=htonl(long_ip_adress); //soket ip adresi atamasi

    sscanf(char_ip_adresi,"%d.%d.%d.%d",&ip_octet_1,&ip_octet_2,&ip_octet_3,&ip_octet_4); //ip adresini tekrar duz ceviriyorum
    sprintf(char_ip_adresi,"%d.%d.%d.%d",ip_octet_4,ip_octet_3,ip_octet_2,ip_octet_1);


    pthread_barrier_wait(&thread_barieri); //threadlerin 100 olana kadar bekleyecekleri barrier noktasi

    socket_file_D=socket(AF_INET,SOCK_STREAM,0); //100 thread oldugunda ayni anda soketleri aciyorlar

    if ((socket_flag=fcntl(socket_file_D,F_GETFL,NULL))<0) //soket getflag
    {
        exit(1);
        printf("Get Socket Flag error");
    };
    if (fcntl(socket_file_D,F_SETFL,socket_flag |O_NONBLOCK)<0) //soketi nonblock olacak sekilde set ediyorum
    {
        exit(1);
        printf("SET Socket Flag error");
    }

    if((socket_baglanti_durumu=connect(socket_file_D,(struct sockaddr *)&client_addr,sizeof(client_addr)))<0) //connect yapıyorum
    {
        if(errno==EINPROGRESS) //eger soket beklemedeyse einprogress veriyor
        {
            fd_set socket_set_ayari;
            FD_ZERO(&socket_set_ayari);
            FD_SET(socket_file_D,&socket_set_ayari);
            socket_baglanti_durumu=select(socket_file_D+1,NULL,&socket_set_ayari,NULL,(struct timeval *)&timeout_struct); //sokete set ayarini atiyor ve time struct ta olusturdugum 2 saniyelik time outu uyguluyorum
        }
    }
    else
    {
        socket_baglanti_durumu=1; //eger soket baglanirsa baglanti durumu 1
    }
    if(fcntl(socket_file_D,F_SETFL,socket_flag)<0) //set edilen nonblock ayarini sifirliyorum
    {
        exit(1);
        printf("Reset Socket Flag error");
    }
    /*ASAGIDAKI KISIMDA THREAD EGER 0 INDEXINE SAHIPSE BEKLEMEDEN DEVAM EDECEK EGER 0 DAN BUYUKSE KENDINDEN BIR ONCEKI THREADI BURADA BEKLEYECEK */
    if(thread_id[0]==pthread_self())
    {
        pthread_join(thread_id[0],NULL);
    }
    else
    {
        for(int i=1; i<100; i++)
        {
            if(thread_id[i]==pthread_self())
            {
                pthread_join(thread_id[i-1],NULL);
            }
        }
    }
    if(socket_baglanti_durumu==1) //onceki threadin yazdirmasini bekledigi icin sira geldiginde portun durumunu yazdiracak eger 1 se aktif degilse kapali
    {
        printf("ip: %s  long ip: %lu\n",char_ip_adresi,long_ip_adress);
        printf("Port:%d \n",ntohs(port_numarasi));
        printf("Soket Durumu: %d \n",socket_file_D);
        printf("Aktif \n");
    }

    else
    {
        printf("ip: %s  long ip: %lu\n",char_ip_adresi,long_ip_adress);
        printf("Port:%d \n",ntohs(port_numarasi));
        printf("Soket Durumu: %d \n",socket_file_D);
        printf("Kapali \n");
    }
    printf("\n");
    close(socket_file_D); //soket kapanir
    return NULL;  //thread biter
}

void subnet_mask_binary_hesaplama()  // subnet degeri kadar diziye 1 atiyor geri kalani 0 yaparak subnet mask adresinin binary dizisini olusturuyorum
{
    int octet=0,deger=0;
    for(int i=1; i<=subnet_mask_degeri; i++)
    {
        binary_subnet_mask[octet][deger]=1;
        deger++;
        if(0==i%8)
        {
            deger=0;
            octet++;
        }
    }
}

void intager_binary_donusumu(int verilen_int_deger,int verilen_int_dizi[8]) // int binary donusumu
{
    for(int i=7; verilen_int_deger>0; i--)
    {
        verilen_int_dizi[i]=verilen_int_deger%2;
        verilen_int_deger=verilen_int_deger/2;
    }
}

void andleme_islemi(int ip_octet[8], int sub_mask_octet[8])  //and islemi bitler ayniysa birakiyor farkliysa 0 yapiyorum
{
    for(int i=0; i<8; i++)
    {
        if(ip_octet[i]!=sub_mask_octet[i])
        {
            ip_octet[i]=0;
        }
    }
}

void binary_degil_islemi(int verilen_octet[8])  // bitlerdeki 1 leri 0 , 0 lari 1 yapiyorum
{
    for(int i=0; i<8; i++)
    {
        if(verilen_octet[i]==1)
        {
            verilen_octet[i]=0;
        }
        else if(verilen_octet[i]==0)
        {
            verilen_octet[i]=1;
        }
    }
}
int binary_intager_donusumu(int atanacak_octet, int verilen_binary_dizi[8])  //binary degerden int degerine ceviriyorum eger 0. bit 1 ise 2^7 yani 128 eger 1. bit 1 ise 2^6 seklinde devam ediyor
{
    atanacak_octet=0;
    for(int i=0; i<8; i++)
    {
        if(i==0&&verilen_binary_dizi[i]==1)
        {
            atanacak_octet+=pow(2,7);
        }
        else if(i==1&&verilen_binary_dizi[i]==1)
        {
            atanacak_octet+=pow(2,6);
        }
        else if(i==2&&verilen_binary_dizi[i]==1)
        {
            atanacak_octet+=pow(2,5);
        }
        else if(i==3&&verilen_binary_dizi[i]==1)
        {
            atanacak_octet+=pow(2,4);
        }
        else if(i==4&&verilen_binary_dizi[i]==1)
        {
            atanacak_octet+=pow(2,3);
        }
        else if(i==5&&verilen_binary_dizi[i]==1)
        {
            atanacak_octet+=pow(2,2);
        }
        else if(i==6&&verilen_binary_dizi[i]==1)
        {
            atanacak_octet+=pow(2,1);
        }
        else if(i==7&&verilen_binary_dizi[i]==1)
        {
            atanacak_octet+=pow(2,0);
        }
    }
    return atanacak_octet;
}
