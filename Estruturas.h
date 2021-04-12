/*
 * GSAP.h (estruturas necessárias no .h)
 * Biblioteca final para comunicação Gsap do esp com o gateway
 * Autor: Enrico Luigi
 * Data: 09/2020
 */
/*
 * Estruturas.H
 * Estruturas usadas pela classe GSAP desenvolvida.
 * Professor orientador: Heitor Medeiros
 * Autor: Enrico Luigi
 * DCA-UFRN
 * Data: 09/2020
 */
#ifndef ESTRUTURAS_H
#define ESTRUTURAS_H
union Union4bytesToUint32  // manipulação de algumas variáveis dos quadros como sessionID
{
  uint32_t myInt;
  byte byteArray[4];
};
union UnionByteToUint_8
{
    uint8_t uint8;
    byte _byte;
};
union Union2bytesToUint_16 // usado para manipulação do networkID e na manipulação da resposta do serviço 03
{
    uint16_t uint16;
    byte byteArray[2];
};
union UnionLongToByte // usado para receber o CRC da função que gera o valor e converter no array para ser passado ao quadro
{
  unsigned long mylong;
  byte myByteArray[4];
};
union Union2ByteToInt_16
{
	int16_t int16;
	byte byteArray[2];
};
struct VisibleString
{
	uint8_t _size;
    byte *text; 
};
struct Device // Estrutura recebida como resposta do serviço referente ao cód.03
{
	byte Address128[16]; 
    uint16_t deviceType;
    byte Address64[8]; 
    byte powerSupplyStatus; 
    byte joinStatus; 
    VisibleString manufacture;
    VisibleString model;
    VisibleString revision;
    VisibleString deviceTag;
    VisibleString serialNo;
};
struct NeighborHealth // Estrutura recebida como resposta do serviço referente ao cód.07
{
	byte Address128[16];
	uint8_t linkstatus;
	uint32_t DPDUsTransmitted;
	uint32_t DPDUsReceived;
	uint32_t DPDUsFailedTransmission;
	uint32_t DPDUsFailedReception;
	int16_t signalStrength;
	uint8_t signalQuality;
};

#endif // ESTRUTURAS_H
