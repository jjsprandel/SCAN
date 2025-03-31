idf_component_register(SRCS "mqtt.c"
                    INCLUDE_DIRS "." "./include"
                    EMBED_TXTFILES ${project_dir}/server_certs/hivemq_cert.pem
                    REQUIRES )