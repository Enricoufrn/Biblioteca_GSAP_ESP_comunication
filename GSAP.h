/*
 * GSAP.h
 * Biblioteca final para comunicação Gsap do esp com o gateway
 * Professor orientador: Heitor Medeiros
 * Autor: Enrico Luigi
 * DCA-UFRN
 * Data: 09/2020
 */
#include <WiFi.h>
#include <Estruturas.h>
#ifndef GSAP_H
#define GSAP_H

class Gsap
{
  public:
    Gsap(WiFiClient *canal_gsap, uint16_t _NetID,char * AddrsGW);
	int abrirCanalSessao();
    int Gsap03();
	void clean03(uint16_t numsDispositivos);
	void clean07();
    int Gsap07(byte *NetworkAddress);
	uint16_t getNumDevices();
	uint16_t getNumLinks();
    void setNetworkID(uint16_t ID);
    void setVersao(byte ver);
	void setHostGTWY(char *_hostGTWY);
	Device *DeviceList;
	NeighborHealth *NeighborHealthList;
  private:
	void setNumDevices(uint16_t);
	void setNumLinks(uint16_t);
	int gsap_connection();
	int Gsap01();
	int Recover03(byte *_QuadroResposta, Device *_deviceList,uint16_t numsDispositivos);
	int Recover07(byte *_QuadroResposta, NeighborHealth *_neighborHealthList,uint16_t numsVizinhos);
    char *hostGTWY;
    int porta, statusSessao, statusCanal,clean,statusMemory;
	uint16_t numLinks,numDevices;
	byte *QuadroResponse;
    byte *QuadroRequest;
    WiFiClient *canal;
    byte versao, serviceCod, sessionPeriod;
    Union4bytesToUint32 transactionID,sessionID,datasize;
    Union2bytesToUint_16 networkID;
    UnionLongToByte unionCRC;
    
};
#endif // GSAP_H
