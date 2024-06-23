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
    int len_datos = 0; //16bits 2bytes largo total del paquete
    BYTE TTL = 0; //8bits 1byte
    BYTE identificacion = 0; //8bits 1byte    si esta fragmentado
    int s_verificacion = 0; //16bits 2bytes
    BYTE ip_origen[4] = {0}; //32bits 4bytes
    BYTE ip_destino[4] = {0}; //32 bits 4bytes
    BYTE DATA[LEN] = {0}; //  memcpy(void*(org),void*(dst),len)
};

int largo_data(frame_ipv4 *frame);

int fcs_IPV4(BYTE *buff);

void empaquetar_IPV4(BYTE *buff, frame_ipv4 *trama);

void desempaquetar_IPV4(BYTE *buff, frame_ipv4 *recuperado);

BYTE buffer[1516] = {0};
bool flag = true;

int main(int nargs, char *arg_arr[]) {
    if (nargs == 4) { //./nodo "nombreIp" Emisor Receptor
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
        printf("|------------chat------------|\n");
        printf("Ya puede escribir sus mensajes !\n");
        printf("Mi IP: %s\n", nombreIP);
        printf("Seleccione IP de destino\n");
        printf("|---SOLO ESCRIBIR ULTIMO VALOR---|\n");
        printf("Recuerde 255 es broadcast\n");
        printf("Rango entre 1-254 usuario\n");
        printf("Ejemplo --> 255 / MENSAJE.\n");
        printf("|----------------------------|\n");


        while (flag) {
            memset(msg, 0, sizeof(msg));
            memset(msg_rx, 0, sizeof(msg_rx));
            //Lee mensajes del puerto virtual y los muestra
            len = readSlip((BYTE *) msg_rx, 1000, vport_lee);
            msg_rx[len - 1] = '\0';
            if (len > 0) { //y si es 255 entonces broadcast
                frame_ipv4 recibido;
                desempaquetar_IPV4((BYTE *) msg_rx, &recibido);
                int fcsRecuperado = fcs_IPV4((BYTE *) msg_rx);

                //-------------------------------------------

                int columnas = 4; // Define el número de columnas para la matriz
                printf("|"); // Imprime el borde izquierdo
                for (int i = 0; i < 16; i += columnas) {
                    for (int j = i; j < i + columnas && j < 16; j++) {
                        printf("%03hhu", msg_rx[j]); // Rellena con ceros a la izquierda hasta tener 3 dígitos
                        if (j < i + columnas - 1 && j < 15) { // Verifica si es el último elemento en la fila
                            printf(" -- ");
                        }
                    }
                    printf("|\n|"); // Imprime el borde derecho y comienza una nueva línea con un borde izquierdo
                }
                printf("\b \b"); // Elimina el último borde izquierdo extra

                //--------------------------------------------------

                if (strcmp((char *) recibido.DATA, "cerrar") == 0) {
                    flag = false;
                }
                //char nIP_destino[16];
                //sprintf(nIP_destino, "%hhu.%hhu.%hhu.%hhu", recibido.ip_destino[0], recibido.ip_destino[1], recibido.ip_destino[2], recibido.ip_destino[3]);
                if (recibido.ip_destino[3] == 255 && (frame.ip_origen[3] != frame.ip_destino[3])) {

                    // Ignora el mensaje si es del mismo nodo
                    if (recibido.ip_origen[3] == frame.ip_origen[3]) {
                        continue;
                    }
                    printf("|-------------------------------------|\n");
                    printf("\n%d Calculado --- %d Recuperado\n", fcsRecuperado, recibido.s_verificacion);
                    printf("MENSAJE BROADCAST RECIBIDO\n");
                    printf("IP ORIGEN.... %hhu.%hhu.%hhu.%hhu\n", recibido.ip_origen[0], recibido.ip_origen[1],
                           recibido.ip_origen[2], recibido.ip_origen[3]);
                    printf("TTL.... %hhu\n", recibido.TTL);
                    printf("MENSAJE.... %s\n", recibido.DATA);
                    printf("|-------------------------------------|\n");
                    recibido.TTL--;
                    if (recibido.TTL > 0 && recibido.ip_origen[3] != frame.ip_origen[3]) {
                        empaquetar_IPV4(buffer, &recibido);
                        writeSlip((BYTE *) buffer, (recibido.len_datos + 16), vport_escribe);
                    } else if (recibido.TTL == 0) {
                        printf("ERROR TTL = 0\n");
                    }
                    memset(buffer, 0, sizeof(buffer));
                } else if (recibido.ip_destino[3] == frame.ip_origen[3] && recibido.ip_destino[3] != 255) {
                    printf("\n |-------------------------------------|\n");

                    printf("%d Calculado --- %d Recuperado\n", fcsRecuperado, recibido.s_verificacion);

                    printf("MENSAJE UNICAST RECIBIDO\n");
                    printf("IP ORIGEN.... %hhu.%hhu.%hhu.%hhu\n", recibido.ip_origen[0], recibido.ip_origen[1],
                           recibido.ip_origen[2], recibido.ip_origen[3]);
                    printf("TTL.... %hhu\n", recibido.TTL);
                    printf("MENSAJE.... %s\n", recibido.DATA);
                    printf("|-------------------------------------|\n");
                } else if (recibido.ip_destino[3] != frame.ip_origen[3]) {
                    recibido.TTL--;
                    empaquetar_IPV4(buffer, &recibido);
                    writeSlip((BYTE *) buffer, (recibido.len_datos + 16), vport_escribe);
                    memset(buffer, 0, sizeof(buffer));
                } else if (recibido.ip_destino[3] == 255 && recibido.ip_origen[3] == frame.ip_origen[3]) {

                    printf("Broadcast enviado a todos los nodos...\n");
                    printf("TTL.... %hhu\n", recibido.TTL);
                }
                printf("|-----------------FRAME---------------|\n");
                printf("flag_fragmento: %hhu\n", recibido.flag_fragmento);
                printf("offset_fragmento: %d\n", recibido.offset_fragmento);
                printf("len_datos: %d\n", recibido.len_datos);
                printf("TTL: %hhu\n", recibido.TTL);
                printf("identificacion: %hhu\n", recibido.identificacion);
                printf("s_verificacion: %d\n", recibido.s_verificacion);
                printf("ip_origen: %hhu.%hhu.%hhu.%hhu\n", recibido.ip_origen[0], recibido.ip_origen[1],
                       recibido.ip_origen[2], recibido.ip_origen[3]);
                printf("ip_destino: %hhu.%hhu.%hhu.%hhu\n", recibido.ip_destino[0], recibido.ip_destino[1],
                       recibido.ip_destino[2], recibido.ip_destino[3]);
                printf("DATA: %s\n", recibido.DATA);
                printf("|-------------------------------------|\n");

                memset(msg_rx, 0, sizeof(msg_rx));
            }

            len = readPort(stdin_desc, (BYTE *) msg, 500, 100);
            msg[len] = '\0';
            if (len > 0) {
                if (msg[len - 1] == '\n') {
                    msg[len - 1] = '\0';
                    len--;
                }
                printf("escribio -> %s\n", msg);
                frame.flag_fragmento = 0;
                frame.offset_fragmento = 0;
                frame.ip_destino[0] = 192;
                frame.ip_destino[1] = 168;
                frame.ip_destino[2] = 130;
                sscanf(msg, "%hhu / %499[^\n]", &frame.ip_destino[3], frame.DATA);
                frame.len_datos = largo_data(&frame);
                frame.TTL = 12;
                frame.identificacion = 0;
                if (frame.ip_destino[3] == 255) {
                    printf("BROADCAST DETECTADO %d -", frame.ip_destino[3]);
                    empaquetar_IPV4(buffer, &frame);
                    writeSlip((BYTE *) buffer, (frame.len_datos + 16), vport_escribe);
                    memset(buffer, 0, sizeof(buffer));
                    printf("ENVIANDO DATA --> %s\n", frame.DATA);
                } else if (frame.ip_destino[3] > 0 && frame.ip_destino[3] < 255) {
                    printf("UNICAST\n");
                    empaquetar_IPV4(buffer, &frame);
                    writeSlip((BYTE *) buffer, (frame.len_datos + 16), vport_escribe);
                    memset(buffer, 0, sizeof(buffer));
                    printf("ENVIANDO DATA --> %s\n", frame.DATA);
                } else {
                    printf("Error\n");
                }
                memset(msg, 0, sizeof(msg));
            }

            //printf("%d\n", strcmp(msg_aux,"255"));


        }
    } else if (nargs == 2 && strcmp(arg_arr[1], HELP) == 0) {
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

int fcs_IPV4(BYTE *buff) {
    int fcs_value = 0;
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            fcs_value += (buff[i] >> j) & 0x01;
        }
    }
    return fcs_value;
}

int largo_data(frame_ipv4 *frame) {
    int i = 0;
    while (frame->DATA[i] != '\0') {
        i++;
    }
    return i;
}

void empaquetar_IPV4(BYTE *buff, frame_ipv4 *trama) {
    int posicion = 0;
    buff[0] |= ((trama->flag_fragmento & 0x0F) << 4) | ((trama->offset_fragmento) & 0x0F);
    buff[1] |= ((trama->offset_fragmento >> 4) & 0xFF);
    posicion += 2;
    memcpy(buff + posicion, &trama->len_datos, 2);
    posicion += 2;
    buff[posicion++] = trama->TTL;
    buff[posicion++] = trama->identificacion;

    posicion += 2;
    memcpy(buff + posicion, trama->ip_origen, sizeof(trama->ip_origen));
    posicion += sizeof(trama->ip_origen);
    memcpy(buff + posicion, trama->ip_destino, sizeof(trama->ip_destino));
    posicion += sizeof(trama->ip_destino);
    memcpy(buff + posicion, trama->DATA, trama->len_datos);

    trama->s_verificacion = fcs_IPV4(buff);
    memcpy(buff + 6, &trama->s_verificacion, 2);

}

void desempaquetar_IPV4(BYTE *buff, frame_ipv4 *recuperado) {
    recuperado->flag_fragmento = (buff[0] >> 4) & 0x0F;
    recuperado->offset_fragmento = (buff[1] & 0xFF) << 4 | (buff[0] & 0x0F);
    int p = 2;
    memcpy(&recuperado->len_datos, buff + p, 2);
    p += 2;
    recuperado->TTL = buff[p++];
    recuperado->identificacion = buff[p++];
    memcpy(&recuperado->s_verificacion, buff + p, 2);
    p += 2;
    memcpy(recuperado->ip_origen, buff + p, sizeof(recuperado->ip_origen));
    p += sizeof(recuperado->ip_origen);
    memcpy(recuperado->ip_destino, buff + p, sizeof(recuperado->ip_destino));
    p += sizeof(recuperado->ip_destino);
    memcpy(recuperado->DATA, buff + p, recuperado->len_datos);
}
