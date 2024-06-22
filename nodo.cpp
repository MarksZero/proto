#include <stdio.h>
#include "serial.h"
#include "slip.h"
#include "string.h"

#define LEN 1500
#define BYTE unsigned char

struct frame_ipv4 {
    BYTE flag_fragmento = 0;
    int offset_fragmento = 0;
    BYTE len_datos[2] = {0};
    BYTE TTL = 0;
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
        char *virtual_port_rx = arg_arr[3];  // Corregido aquí

        FILE *vport_escribe = fopen(virtual_port_tx, "w");
        FILE *vport_lee = fopen(virtual_port_rx, "r");

        int stdin_desc = fileno(stdin);

        printf("chat\n");
        printf("Ya puede escribir sus mensajes!\n");
        printf("Mi IP: %s\n", nombreIP);
        printf("Seleccione nodo de destino (1-3) o 255 para broadcast\n");
        printf("Ejemplo --> 2 / MENSAJE.\n");

        while (true) {
            // Lee mensajes del puerto virtual
            len = readSlip((BYTE *)&received_frame, sizeof(frame_ipv4), vport_lee);
            if (len > 0) {
                if (received_frame.ip_destino[3] == frame.ip_origen[3] || received_frame.ip_destino[3] == 255) {
                    printf("\n------------------------------------------\n");
                    printf("Mensaje recibido de 192.168.130.%d: %s\n",
                           received_frame.ip_origen[3], received_frame.DATA);
                } else {
                    // Reenvía el mensaje al siguiente nodo
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