#include <stdio.h>
#include "serial.h"
#include "slip.h"
#include "string.h"

#define LEN 1500
#define BYTE unsigned char
#define HELP "-h"

// #define VIRTUAL_PORT "../tmp/p1"
struct frame_ipv4 {
    BYTE flag_fragmento = 0; //4bits  0 no fragmentado, 1 fragmentado, 2 ultimo fragmento
    int offset_fragmento = 0; //12bits   si esta fragmentado
    BYTE len_datos[2] = {0}; //16bits 2bytes largo total del paquete
    BYTE TTL = 0; //8bits 1byte
    BYTE identificacion = 0; //8bits 1byte    si esta fragmentado
    BYTE s_verificacion[2] = {0}; //16bits 2bytes
    BYTE ip_origen[4] = {0}; //32bits 4bytes
    BYTE ip_destino[4] = {0}; //32 bits 4bytes
    BYTE DATA[LEN] = {0}; //  memcpy(void*(org),void*(dst),len)
};

char buff[1516] = {0};



int main(int nargs, char *arg_arr[]) {
    if (nargs == 4) {
        //./nodo "nombreIp" Emisor Receptor
        char msg[500];
        char msg_rx[1000];

        frame_ipv4 frame;
        int len = 0;
        //Obtiene nombre IP
        char *nombreIP = arg_arr[1];
        sscanf(nombreIP, "%hhu.%hhu.%hhu.%hhu", &frame.ip_origen[0], &frame.ip_origen[1], &frame.ip_origen[2],
               &frame.ip_origen[3]);

        //Obtiene puerto virtual tx
        char *virtual_port_tx = arg_arr[2];
        //Obtiene puerto virtual rx
        char *virtual_port_rx = arg_arr[3];

        FILE *vport_escribe = fopen(virtual_port_tx, "w");
        FILE *vport_lee = fopen(virtual_port_rx, "r");
        //Inicia la capa 1 y 2
        //Obtiene descriptores
        int stdin_desc = fileno(stdin);
        //Obtiene descriptores

        //Indica inicio del chat
        printf("chat\n");
        printf("Ya puede escribir sus mensajes !\n");
        printf("Mi IP: %s\n", nombreIP);
        printf("Seleccione IP de destino\n");
        printf("|---SOLO ESCRIBIR ULTIMO VALOR---|\n");
        printf("Recuerde 255 es broadcast\n");
        printf("Rango entre 1-254 usuario\n");
        printf("Ejemplo --> 255 / MENSAJE.\n");
        printf("|--------------------------------|\n");

        while (true) {
            //Lee mensajes del puerto virtual y los muestra
            len = readSlip((BYTE *) msg_rx, 1000, vport_lee);
            msg_rx[len - 1] = '\0';
            if (len > 0) {
                printf("\n------------------------------------------\n");
                printf("El otro usuario dice: %s  \n", msg_rx);


            }


            len = readPort(stdin_desc, (BYTE *) msg, 500, 100);
            msg[len] = '\0';

            if (len > 0) {
                if (msg[len - 1] == '\n') {
                    msg[len - 1] = '\0';
                    len--;
                }
                printf("escribio -> %s\n", msg);
                frame.ip_destino[0] = 192;
                frame.ip_destino[1] = 168;
                frame.ip_destino[2] = 130;
                sscanf(msg, "%hhu / %499[^\n]", &frame.ip_destino[3], frame.DATA);
                writeSlip((BYTE *) msg, len, vport_escribe);
                if (frame.ip_destino[3] == 255) {
                    printf("BROADCAST DETECTADO %d  ", frame.ip_destino[3]);
                    printf("\n");
                    printf("ENVIANDO DATA --> %s\n", frame.DATA);
                } else if (frame.ip_destino[3] > 0 && frame.ip_destino[3] < 255) {
                    printf("UNICAST\n");
                    printf("ENVIANDO DATA --> %s\n", frame.DATA);
                } else {
                    printf("Error\n");
                }
            }

            //printf("%d\n", strcmp(msg_aux,"255"));
        }
    }
    if (nargs == 2 && strcmp(arg_arr[1],HELP) == 0) {
        printf("MANUAL DE USUARIO:\n");
        printf("-------------------\n");
        printf("\t Modo de Uso:\n");
        printf("\t\t1-.\t./nodo nombreIP emisor receptor\n");
        printf("\t Ejemplo:\n");
        printf("\t\t1-.\t./nodo 192.168.130.1 ../tmp/p1 ../tmp/p10\n");
    } else {
        //Se requiere un y solo un argumento; el puerto virtual
        printf("Debe indicar el puerto virtual y/o IP!\n");
        printf("Para mas ayuda escriba ./nodo -h\n");
    }
    return 0;
}
