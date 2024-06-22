#include <stdio.h>
#include "serial.h"
#include "slip.h"
#include "string.h"

#define LEN 1500
#define BYTE unsigned char
#define MAX_TTL 3

struct frame_ipv4 {
    BYTE flag_fragmento = 0;
    int offset_fragmento = 0;
    BYTE len_datos[2] = {0};
    BYTE TTL = MAX_TTL;
    BYTE identificacion = 0;
    BYTE s_verificacion[2] = {0};
    BYTE ip_origen[4] = {0};
    BYTE ip_destino[4] = {0};
    BYTE DATA[LEN] = {0};
};

int main(int nargs, char *arg_arr[]) {
    if (nargs == 4) {
        char msg[500];
        frame_ipv4 frame, received_frame;
        int len = 0;

        // Obtiene nombre IP
        char *nombreIP = arg_arr[1];
        sscanf(nombreIP, "%hhu.%hhu.%hhu.%hhu", &frame.ip_origen[0], &frame.ip_origen[1], &frame.ip_origen[2], &frame.ip_origen[3]);

        // Obtiene puertos virtuales
        char *virtual_port_tx = arg_arr[2];
        char *virtual_port_rx = arg_arr[3];

        FILE *vport_escribe = fopen(virtual_port_tx, "w");
        FILE *vport_lee = fopen(virtual_port_rx, "r");

        int stdin_desc = fileno(stdin);

        printf("|------------chat------------|\n");
        printf("Ya puede escribir sus mensajes!\n");
        printf("Mi IP: %s\n", nombreIP);
        printf("Seleccione nodo de destino (1-3) o 255 para broadcast\n");
        printf("Ejemplo --> 2 / MENSAJE.\n");
        printf("|----------------------------|\n);

        while (true) {
            // Lee mensajes del puerto virtual
            len = readSlip((BYTE *)&received_frame, sizeof(frame_ipv4), vport_lee);
            if (len > 0) {
                bool es_uni = (received_frame.ip_destino[3] == frame.ip_origen[3]);
                bool es_broadcast = (received_frame.ip_destino[3] == 255);             // Verifica si el mensaje es un broadcast

                if (es_uni || es_broadcast) {
                    printf("\n------------------------------------------\n");
                    printf("Mensaje de 192.168.130.%d: %s\n",
                           received_frame.ip_origen[3], received_frame.DATA);
                }

                // Reenvía el mensaje si no es para este nodo y el TTL > 0
                if (!es_uni && received_frame.TTL > 0) {
                    received_frame.TTL--;
                    writeSlip((BYTE *)&received_frame, sizeof(frame_ipv4), vport_escribe);
                }
            }

            // Lee entrada del usuario
            len = readPort(stdin_desc, (BYTE *)msg, 500, 100);
            if (len > 0) {
                if (msg[len - 1] == '\n') {
                    msg[len - 1] = '\0';
                    len--;
                }
                printf("escribio -> %s\n", msg);

                sscanf(msg, "%hhu / %499[^\n]", &frame.ip_destino[3], frame.DATA);
                frame.TTL = MAX_TTL;

                writeSlip((BYTE *)&frame, sizeof(frame_ipv4), vport_escribe);

                if (frame.ip_destino[3] == 255) {
                    printf("BROADCAST DETECTADO\n");
                    printf("ENVIANDO DATA --> %s\n", frame.DATA);
                } else if (frame.ip_destino[3] >= 1 && frame.ip_destino[3] <= 3) {
                    printf("UNICAST al nodo %d\n", frame.ip_destino[3]);
                    printf("ENVIANDO DATA --> %s\n", frame.DATA);
                } else {
                    printf("Error: Nodo destino inválido\n");
                }
            }
        }
    }
    return 0;
}