---
layout: default
title: Industry Pack Bridge
nav_order: 4
---


Industry Pack Bridge
====================

1. Introduction
---------------

The Industry Pack Bridge appears as a 16-bit master component in an Altera Avalon Bus SOPC system. Only 8 MHz operation is supported.

2. Industry Pack Bus Transactions
---------------------------------

### 2.1 ID Prom

The 16-bit ID Prom is initialized with the contents of the `IDProm.mif` memory initialization file. A script is available to compute the CRC value to be loaded into this file.

### 2.2 I/O (Register) Space

Industry Pack I/O space accesses are mapped to Avalon addresses 000000 through 00007F.

### 2.3 Memory Space

Industry Pack memory space accesses are mapped to Avalon addresses 800000 through FFFFFF.

### 2.4 Interrupts

All Avalon interrupts are funneled through the module IntReq0\* line. The interrupt vector is the 8 bit sum of the value on the VectorBase input lines and the Avalon interrupt number.

### 2.5 DMA

Industry Pack DMA operation is not supported.

3. ID PROM Modification
-----------------------

To make changes to the ID PROM perform these steps

1. Copy the PROM initialization file IDProm.mif to your application source directory `cp ...../IDProm.mif MyProm.mif`
2. Make your changes 
    - Locations 03 and 04 are the manufacturer ID code
    - Location 05 is the model number
    - Location 06 is the revision number
    - Locations 08 and 09 are the driver ID code
    - Location 0B is the number of checksummed bytes in the ID PROM header
    - Location 0C is the CRC
3. To compute the CRC 
    - Copy the `crc.c` program to your application directory `cp ...../crc.c crc.c`
    - Compile it (`cc -o crc crc.c`)
    - Run your memory initialization file through the crc program `./crc <MyProm.mif >IDProm.mif`
    - Use Quartus to compile your application
