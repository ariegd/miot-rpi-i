# Embed CA, certificate & key directly into binary
idf_component_register(SRCS "coapc_comp.c"
                    INCLUDE_DIRS "." "include"
                    EMBED_TXTFILES certs/coap_ca.pem certs/coap_client.crt certs/coap_client.key oscore/coap_oscore.conf
                    REQUIRES esp_wifi esp_event esp_netif nvs_flash coap)
