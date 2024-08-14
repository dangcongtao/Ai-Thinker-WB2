/**
 * @file main.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-10-09
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
#include <aos/yloop.h>
#include <aos/kernel.h>
#include <lwip/tcpip.h>
#include <wifi_mgmr_ext.h>
#include <hal_wifi.h>
#include <lwip/netif.h>
#include <lwip/inet.h>
#include <blog.h>

#include "FreeRTOS.h"
#include "portmacro.h"
#include "event_groups.h"
#include "lwip/err.h"
#include "string.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/api.h"
#include "web_server.h"

#define AP_SSID "taodc"
#define AP_PWD "66668888"

#define TAG "softAP"
#define CARRIAGE_RETURN 13
#define SERVER_PORT 80

#define ROUTER_SSID "RD_Hunonic_Mesh"
#define ROUTER_PWD "66668888"

const static char http_html_hdr[] =
    "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char http_index_hml[] = "<!DOCTYPE html>"
                                     "<html>\n"
                                     "<head>\n"
                                     "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                                     "  <style type=\"text/css\">\n"
                                     "    html, body, iframe { margin: 0; padding: 0; height: 100%; }\n"
                                     "    iframe { display: block; width: 100%; border: none; }\n"
                                     "  </style>\n"
                                     "<title>HELLO Ai-WB2 module</title>\n"
                                     "</head>\n"
                                     "<body>\n"
                                     "<h1>Hello World, from Ai-WB2 module</h1>\n"
                                     "</body>\n"
                                     "</html>\n";

static uint8_t s_flag_start_ap = false;
static uint8_t s_flag_stop_ap = false;
/* */
static uint8_t s_flag_start_sta = false;
static uint8_t s_flag_stop_sta = false;
static uint32_t s_time = 0;
static uint32_t s_time_2 = 0;
static uint32_t s_time_start_ap = 0;

static wifi_interface_t s_wifi_interface;

/* *************** STA *************** */
static void wifi_sta_connect(char* ssid, char* password)
{
    blog_info("will connect to wifi: [%s] - pass: [%s] ", ssid, password);
    

    s_wifi_interface = wifi_mgmr_sta_enable();
    wifi_mgmr_sta_connect(s_wifi_interface, ssid, password, NULL, NULL, 0, 0);
}

static void wifi_sta_stop()
{
    blog_info("will deinit wifi sta ");

    wifi_mgmr_sta_autoconnect_disable();
    wifi_mgmr_sta_disconnect();
    wifi_mgmr_sta_disable(s_wifi_interface);
}

static void request_stop_ap(void)
{
    s_flag_stop_ap = true;
}

static void request_start_ap(void)
{
    s_flag_start_ap = true;
}

static void request_start_sta(void)
{
    s_flag_start_sta = true;
}

static void request_stop_sta(void)
{
    s_flag_stop_sta = true;
}

static void get_body_msg(char *msg)
{
    char *token = NULL;
    token = strtok(msg, "\n");
    while (token[0] != CARRIAGE_RETURN)
    {
        token = strtok(NULL, "\n");
    }
    token = strtok(NULL, "\n");
    if (token == NULL)
    {
        blog_error("no body");
        return;
    }
    blog_warn("token [%s] ", token);
    if (strstr(token, "ssid") != NULL)
    {
        blog_info("will connect wifi ");
        request_stop_ap();
        s_time = aos_now_ms();
    }
}

static void web_http_server(struct netconn *conn)
{
    struct netbuf *inputbuf;
    char *buf;
    u16_t buflen;
    err_t err;

    err = netconn_recv(conn, &inputbuf);
    if (err == ERR_OK)
    {
        netbuf_data(inputbuf, (void **)&buf, &buflen);
        blog_warn("the received data:\n[%s]\n", buf);
        get_body_msg(buf);

        /* Judge if this is an HTTP GET command */
        if (buflen >= 5 && buf[0] == 'G' && buf[1] == 'E' && buf[2] == 'T' && buf[3] == ' ' && buf[4] == '/')
        {
            netconn_write(conn, http_html_hdr, sizeof(http_html_hdr) - 1, NETCONN_NOCOPY);

            if (buf[5] == 'h')
            {
                netconn_write(conn, http_index_hml, sizeof(http_index_hml) - 1, NETCONN_NOCOPY);
            }
            else if (buf[5] == 'l')
            {
                netconn_write(conn, http_index_hml, sizeof(http_index_hml) - 1, NETCONN_NOCOPY);
            }
            else
            {
                netconn_write(conn, http_index_hml, sizeof(http_index_hml) - 1, NETCONN_NOCOPY);
            }
        }
    }
    netconn_close(conn);
    netbuf_delete(inputbuf);
}

void http_server_start(void *pvParameters)
{
    struct netconn *conn, *newconn;
    err_t err;
    conn = netconn_new(NETCONN_TCP);
    err = netconn_bind(conn, NULL, SERVER_PORT);
    if (err != ERR_OK)
    {
        blog_error("fail to bind ");
    }
    else
    {
        blog_info("bind success ");
    }
    netconn_listen(conn);
    while (1)
    {
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK)
        {
            web_http_server(newconn);
            netconn_delete(newconn);
        }
        else
        {
            netconn_close(conn);
            netconn_delete(conn);
            break;
        }
    }
}



static wifi_conf_t ap_conf = {
    .country_code = "CN",
};
static wifi_interface_t ap_interface;
/**
 * @brief wifi_ap_ip_set
 *      Set the IP address of soft AP
 * @param ip_addr IPV4 addr
 * @param netmask netmask
 * @param gw DNS
 */
static void wifi_ap_ip_set(char* ip_addr, char* netmask, char* gw)
{
    struct netif* ap_netif = netif_find("ap1");
    int i = 0;
    int ap_ipaddr[4] = { 0 };
    int ap_netmask[4] = { 255,255,255,0 };
    int ap_gw_arry[4] = { 0,0,0,0 };

    ap_ipaddr[0] = atoi(strtok(ip_addr, "."));

    for (i = 1;i<4;i++) {
        ap_ipaddr[i] = atoi(strtok(NULL, "."));
    }
    if (netmask) {
        ap_netmask[0] = atoi(strtok(netmask, "."));
        for (i = 1;i<4;i++)
            ap_netmask[i] = atoi(strtok(NULL, "."));
    }
    if (gw) {
        ap_gw_arry[0] = atoi(strtok(gw, "."));
        for (i = 1;i<4;i++)
            ap_gw_arry[i] = atoi(strtok(NULL, "."));
    }

    if (ap_netif) {

        ip_addr_t ap_ip;
        ip_addr_t ap_mask;
        ip_addr_t ap_gw;
        IP4_ADDR(&ap_ip, ap_ipaddr[0], ap_ipaddr[1], ap_ipaddr[2], ap_ipaddr[3]);
        IP4_ADDR(&ap_mask, ap_netmask[0], ap_netmask[1], ap_netmask[2], ap_netmask[3]);
        IP4_ADDR(&ap_gw, ap_gw_arry[0], ap_gw_arry[1], ap_gw_arry[2], ap_gw_arry[3]);

        netif_set_down(ap_netif);
        netif_set_ipaddr(ap_netif, &ap_ip);
        netif_set_netmask(ap_netif, &ap_mask);
        netif_set_gw(ap_netif, &ap_gw);
        netif_set_up(ap_netif);
        blog_info("[softAP]:SSID:%s,PASSWORD:%s,IP addr:%s", AP_SSID, AP_PWD, ip4addr_ntoa(netif_ip4_addr(ap_netif)));
    }
    else
        blog_info("no find netif ap1 ");

}

/**
 * @brief wifi_ap_start
 *
 */
static void wifi_ap_start()
{
    ap_interface = wifi_mgmr_ap_enable();
    wifi_mgmr_conf_max_sta(4);
    wifi_mgmr_ap_start(ap_interface, AP_SSID, 0, AP_PWD, 6);
    wifi_ap_ip_set("192.168.169.1", "255.255.255.0", "192.168.169.1");  //defaut gateway ip is "192.168.169.1",if you want usb other gateway ip ,please change components/network/lwip_dhcpd/dhcp_server_raw.c：42   DHCPD_SERVER_IP 
                                                                        //for example, gateway ip:"192.168.4.1" , change DHCPD_SERVER_IP to "192.168.4.1"  :
                                                                        //wifi_ap_ip_set("192.168.4.1", "255.255.255.0", "192.168.4.1");
                                                                        //components/network/lwip_dhcpd/dhcp_server_raw.c：42   #define DHCPD_SERVER_IP "192.168.4.1"
}

static void wifi_ap_stop()
{
    wifi_mgmr_ap_stop(ap_interface);
}

/* will change name this callback */
static void event_cb_wifi_event(input_event_t* event, void* private_data)
{
    switch (event->code) {
        case CODE_WIFI_ON_INIT_DONE:
            blog_info("<<<<<<<<<  init wifi done  <<<<<<<<<<");
            wifi_mgmr_start_background(&ap_conf);
            break;
        case CODE_WIFI_ON_MGMR_DONE:
            blog_info("<<<<<<<<< startting soft ap <<<<<<<<<<<");
            wifi_ap_start();
            break;
        case CODE_WIFI_ON_AP_STARTED:
        {
            blog_info("<<<<<<<<< startt soft ap OK<<<<<<<<<<<");
            /* start http server */
            xTaskCreate(http_server_start, (char *)"http server", 1024 * 4, NULL, 15, NULL);
        } break;
        case CODE_WIFI_ON_AP_STOPPED:
            break;
        case CODE_WIFI_ON_AP_STA_ADD:
        {
            blog_info("<<<<<<<<< station connent ap <<<<<<<<<<<");
            /* device connect wifi */
        } break;
        case CODE_WIFI_ON_AP_STA_DEL:
            blog_info("<<<<<<<<< station disconnet ap <<<<<<<<<<<");

            break;

        case CODE_WIFI_ON_CONNECTED:
        {
            blog_info("wifi sta connected");
            s_time_2 = aos_now_ms();
        }
        break;

        default:
            break;

    }
}

static void proc_main_entry(void* pvParameters)
{

    /* will change name this callback */
    aos_register_event_filter(EV_WIFI, event_cb_wifi_event, NULL);
    hal_wifi_start_firmware_task();
    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);
    vTaskDelete(NULL);
}

static void system_thread_init()
{
    /*nothing here*/
}

static void main_task(void* pvParameters)
{
    while (true)
    {
        if (s_flag_stop_ap)
        {
            s_flag_stop_ap = false;
            blog_info("wanna stop ap");
            wifi_ap_stop();
        }
        if (s_flag_start_ap)
        {
            s_flag_start_ap = false;
            blog_info("wanna start ap ");
            wifi_ap_start();
        }

        if (s_flag_start_sta)
        {
            s_flag_start_sta = false;
            wifi_sta_connect(ROUTER_SSID, ROUTER_PWD);
        }

        if (s_flag_stop_sta)
        {
            s_flag_stop_sta = false;
            wifi_sta_stop();
        }

        if (aos_now_ms()-s_time > 3000 && s_time)
        {
            s_time = 0;
            request_start_sta();
            blog_info("timer act ");
        }

        if (aos_now_ms()-s_time_2 > 3000 && s_time_2)
        {
            s_time_2 = 0;
            request_stop_sta();
            blog_info("time2 act ");
        }


        vTaskDelay(200);
    }
}
void main()
{
    system_thread_init();

    puts("[OS] Starting TCP/IP Stack...");
    tcpip_init(NULL, NULL);
    puts("[OS] proc_main_entry task...");
    xTaskCreate(proc_main_entry, (char*)"main_entry", 1024, NULL, 15, NULL);

    xTaskCreate(main_task, (char*)"main_entry", 1024, NULL, 1, NULL);
}
