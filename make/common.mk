SUDO ?= sudo
PIP ?= pip3
IDF := idf.py
PORT := /dev/ttyUSB0
ESP_IDF_TARGET ?= esp32s3


PROJECT_NAME ?= example


PROJECT_DIR := $(shell pwd)
MAIN_DIR := $(PROJECT_DIR)/main
COMPONENT_DIR := $(PROJECT_DIR)/components
LIBS_DIR := $(PROJECT_DIR)/libs
BUILD_DIR := $(PROJECT_DIR)/build
REQUIREMENTS_DIR := $(PROJECT_DIR)/requirements
