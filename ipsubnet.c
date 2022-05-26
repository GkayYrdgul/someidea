#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char char_ip_adresi[50]; //Girilen ip adresi
int subnet_mask_degeri; //Girilen Subnet Mask değeri
int ip_octet_1=0,ip_octet_2=0,ip_octet_3=0,ip_octet_4=0; //İp değerinin octetler halindeki decimal degeri
int submask_octet_1=0,submask_octet_2=0,submask_octet_3=0,submask_octet_4=0; //subnet mask degerinin octetler halindeki decimal degeri
int binary_ip_adresi[4][8]; //ip adresinin binary hali
int binary_subnet_mask[4][8]; // Subnet mask degerinin binary hali
int host_baslangic_adresi=0, host_bitis_adresi=0; //baslangic adresi network adresinin 4. octetinin 1 fazlasi bitis adresi ise Broadcast adresinin 1 eksigi
int host_sayisi=0; //Normal halindeki host sayisi
int submask24_network_sayisi=0; //24 subnetlere bolundugunde ki host sayisi

void intager_binary_donusumu(int verilen_int_deger,int verilen_int_dizi[8]); //intager degeri binary degerine donusturen fonksiyon
void subnet_mask_binary_hesaplama();                             //verilen subnet degerini binary seklinde listeye atayan fonksiyon
void andleme_islemi(int ip_octet[8], int sub_mask_octet[8]);     //binary degerleri arasinda and isleminin yapildigi fonksiyon
void binary_degil_islemi(int verilen_octet[8]);                  //binary degerleri tersine ceviren Not yani degil fonksiyonu
int binary_intager_donusumu(int atanacak_octet, int verilen_binary_dizi[8]); //binary diziden intager sayiya donusturen fonksiyon

int main (int argc, char *argv[])
{
    if(argc!=3)
    {
        printf("./ipsubnet [ip] [mask] \n");
        exit(0);
    }
    strcpy(char_ip_adresi,argv[1]);
    sscanf(char_ip_adresi,"%d.%d.%d.%d",&ip_octet_1,&ip_octet_2,&ip_octet_3,&ip_octet_4);
    subnet_mask_degeri=atoi(argv[2]);

    if(subnet_mask_degeri<=0||subnet_mask_degeri>24)
    {
        perror("Hatali Deger");
        exit(1);
    }
    if(ip_octet_1<=0||ip_octet_1>255)
    {
        perror("Hatali Deger");
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

    host_bitis_adresi=ip_octet_4-1;   //Broadcast adresinin 4. octetinin 1 eksigi son host adresi

    printf("\nHost Bitis Adresi: %d.%d.%d.%d \n",ip_octet_1,ip_octet_2,ip_octet_3,host_bitis_adresi);

    printf("\nBroadcast Adresi: %d.%d.%d.%d \n \n",ip_octet_1,ip_octet_2,ip_octet_3,ip_octet_4);

    printf("Host Sayisi= %d\n \n",host_sayisi);

    printf("/24 Network Sayisi:%d \n\n",submask24_network_sayisi);
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
