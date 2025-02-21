# Embed the server root certificate into the final binary
idf_build_get_property(project_dir PROJECT_DIR)
idf_component_register(SRCS "firebase_http_client.c" "firebase_utils.c"
                    INCLUDE_DIRS "."
                    EMBED_TXTFILES ${project_dir}/server_certs/firebase_cert.pem
                    REQUIRES esp_http_client mbedtls esp-tls
                    PRIV_REQUIRES esp_event efuse)