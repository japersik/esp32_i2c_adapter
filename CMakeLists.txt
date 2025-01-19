cmake_minimum_required(VERSION 3.16)

idf_component_register(SRCS "src/i2c_adapter.c" 
                       	INCLUDE_DIRS "include"
												PRIV_REQUIRES driver 
)

