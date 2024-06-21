#include <stdio.h>
#include <string.h>
#define BYTE unsigned char
#define LEN 1500
struct frame_ipv4
{
    BYTE flag_fragmento; //4bits  0 no fragmentado, 1 fragmentado, 2 ultimo fragmento
    int offset_fragmento=0; //12bits   si esta fragmentado
    BYTE len_datos[2]; //16bits 2bytes largo total del paquete 
    BYTE TTL; //8bits 1byte
    BYTE identificacion; //8bits 1byte    si esta fragmentado
    BYTE s_verificacion[2]; //16bits 2bytes
    BYTE ip_origen[4]; //32bits 4bytes
    BYTE ip_destino[4]; //32 bits 4bytes
    BYTE DATA[LEN]; //  memcpy(void*(org),void*(dst),len) max 65536 bytes pero se usaran 1500
};


int main(int nargs, char* argv[]){
    char* nombreIP = argv[1];
    printf("nIP== %s\n", nombreIP);
    frame_ipv4 buff;
    sscanf(nombreIP, "%hhu.%hhu.%hhu.%hhu", &buff.ip_origen[0], &buff.ip_origen[1], &buff.ip_origen[2], &buff.ip_origen[3]);
    printf("ip 1obtenida== %d\n", buff.ip_origen[0]);
    printf("ip 2obtenida== %d\n", buff.ip_origen[1]);
    printf("ip 3obtenida== %d\n", buff.ip_origen[2]);
    printf("ip 4obtenida== %d\n", buff.ip_origen[3]);
    /*
    BYTE ip[4];
    sscanf(nombreIP, "%hhu.%hhu.%hhu.%hhu", &ip[0], &ip[1], &ip[2], &ip[3]);
    printf("ip 1obtenida== %d\n", ip[0]);
    printf("ip 2obtenida== %d\n", ip[1]);
    printf("ip 3obtenida== %d\n", ip[2]);
    printf("ip 4obtenida== %d\n", ip[3]);
    char nIP[16];
    sprintf(nIP, "%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
    printf("ip recuperada == %s\n", nIP);

    printf("%d\n",strcmp(nIP,nombreIP));
    */
    
    

    return 0;
}
