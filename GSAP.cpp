/*
 * GSAP.cpp
 * Biblioteca desenvolvida para a comunicação pelo padrão GSAP utilizando o ESP-32 como controlador da rede
 * Professor orientador: Heitor Medeiros
 * Autor: Enrico Luigi
 * DCA-UFRN
 * Data: 09/2020
 */
#include <GSAP.h>
#include <WiFi.h>
#include <Estruturas.h>
// função para abertura da do canal de comunicação com o gateway 
int Gsap :: gsap_connection()
{   
  if (!canal->connected()) {
    canal->stop();
    if (!canal->connect(hostGTWY, porta)){
      return -1;
    }
  } else{
    delay(5);
    return 0;
  }
}

unsigned long crc32c (byte *code, int bytes) // crc usado nos envios do quadros
{
  unsigned long Byte, mask;
  unsigned long crc = 0xFFFFFFFF;
  int index, shift;

  for ( index = 0 ; index < bytes ; index++ )
  {
    Byte = code[index];
    // XOR with Next Byte
    crc = crc ^ Byte;
    // Shift
    for (shift = 7; shift >= 0; shift--)
    {
      mask = -(crc & 1);
      crc = (crc >> 1) ^ (0x82F63B78 & mask);
    }
  }
  return ~crc;
}
// construtor já faz a requisição da sessão (cod_01) e seta a versão do equipamento("0xF0"), porta para se conectar com o GATEWAY e o ID da rede
Gsap :: Gsap(WiFiClient *canal_gsap, uint16_t _NetID,char * AddrsGW)
{
    statusSessao = 2; // alterado para 0 quando a sessão GSAP for iniciada
	statusCanal = 1; // alterado para 0 quando o canal de comunicação com o gateway for criado
	statusMemory = 0; // alterado para 1 sempre que é feito o uso de alguma requisição que faz o uso de alocação dinâmica de memória
	clean = 0; // usado parar saber se os precessos de liberação da memória dévidos foram feitos
	canal = canal_gsap; // um ponteiro que aponte para váriavel que foi usada para armazenar o objeto da classe WiFiClient como parámetro necessário no construtor da classe, sendo igualado a "canal" sendo esse último o que será usado dentro da classe
    setVersao(0xF0);
	setNetworkID(_NetID);
	setHostGTWY(AddrsGW);
	setNumLinks(0);
	setNumDevices(0);
	porta = 4902;	
}
int Gsap :: abrirCanalSessao() // método a ser usado antes de se fazer o uso de qualquer requisição ao gateway referente ao padrão GSAP
{
	/*
	 Nesse método é feito o uso da função de conexão com o gateway e o a requisição da abertura da sessão GSAP com o mesmo
	*/
	statusCanal = gsap_connection();
	statusSessao = Gsap01();
	if(statusCanal==0){
		if(statusSessao==0){
			return 0;
		}else{
			return statusSessao;
		}		
	}else{
		return statusCanal; 
	}
}
void Gsap :: setNetworkID(uint16_t ID) // método usado para setar o ID da rede
{
  networkID.uint16 = ID;
}
void Gsap :: setVersao(byte ver) // método usado para setar a versão do equipamento
{
  versao = ver;
}
void Gsap :: setHostGTWY(char *_hostGTWY){
	hostGTWY = _hostGTWY; // o ponteiro do que aponta para o endereço do host é passado como parámetro ao construtor que utiliza esse método para setar o valor do endereço do host dentro da classe
}
void Gsap :: setNumLinks(uint16_t nl){
	numLinks = nl;
}
void Gsap :: setNumDevices(uint16_t nd){
	numDevices = nd;
}
uint16_t Gsap :: getNumLinks(){
	return numLinks;
}
uint16_t Gsap :: getNumDevices(){
	return numDevices;
}
int Gsap :: Gsap01() // Requisição da abertura da sessão GSAP, deve ser usado feito antes de qualquer requisição de qualquer serviço suportado pelo padrão
{
	/*
	 É feita uma alocação para armazenar os quadros que serão enviados, os quadros são construidos e enviado e logo após é feito a liberação da memória usada.
	 É um método private pois o trabalho da abertura da sessão é feito junto com a criação do canal de comunicação no método "abreCanalSessao"
	*/
	if(statusCanal==0)
	{
		QuadroRequest = new byte[28];
		QuadroResponse = new byte[27];
		if(QuadroResponse == 0 || QuadroRequest == 0){
			return -11;
		}else{
			byte aux[6];
			// setar valores da variaveis do meu quadro
			// seta o valor do serviceCod e passa para o quadro
			serviceCod = 0x01;
			// seta o valor do sessionID em zero(criar id de sessão)
			sessionID.myInt = 0x00;
			// seta transactionID
			transactionID.myInt = 0x01;
			// seta datasize
			datasize.myInt = 0x0A;
			// seta periodo da sessão
			sessionPeriod = 0xFF;
			// networkID já foi setado
			// nesse ponto as variaveis do meu quadro já estão prontas logo posso montar o quadro
			QuadroRequest[0] = versao;
			QuadroRequest[1] = serviceCod;
			// passa o bytes referente ao id da sessão para o quadro
			for(int i = 0; i < 4;i++)
			{
				QuadroRequest[i+2] = sessionID.byteArray[i];
			}
			// passa o bytes referente ao id da transação para o quadro
			for(int i = 0; i < 4;i++)
			{
				QuadroRequest[i+6] = transactionID.byteArray[3-i];
			}
			// passa o bytes referente ao datasize para o quadro
			for(int i = 0; i < 4;i++)
			{
				QuadroRequest[i+10] = datasize.byteArray[3-i];
			}
			// passa o bytes referente ao headerCRC para o quadro
			unionCRC.mylong = crc32c(QuadroRequest,14);
			for(int i = 0; i < 4;i++)
			{
				QuadroRequest[i+14] = unionCRC.myByteArray[i];
			}
			// passa o bytes referente ao sessionPeriod para o quadro
			for(int i = 0; i < 4;i++)
			{
				QuadroRequest[i+18] = sessionPeriod;
			}
			// passa o bytes referente ao networkID para o quadro
			for(int i = 0; i < 2;i++)
			{
				QuadroRequest[i+22] = networkID.byteArray[1-i];
			}
			// passa o bytes referente ao headerCRC para o quadro
			for(int i=0;i<6;i++)
			{
				aux[i] = QuadroRequest[i+18]; // inicialização do vetor auxiliar para ser usado para o calculo do crc
			}
			unionCRC.mylong = crc32c(aux,6);
			for(int i = 0; i < 4;i++)
			{
				QuadroRequest[i+24] = unionCRC.myByteArray[i];
			}
			canal->write(QuadroRequest,28);
			canal->readBytes(QuadroResponse,27);
			if(QuadroResponse[1] != 0x81)
			{
			  return -22;
			}
			else
			{
			  sessionID.byteArray[0] = QuadroResponse[2];
			  sessionID.byteArray[1] = QuadroResponse[3];
			  sessionID.byteArray[2] = QuadroResponse[4];
			  sessionID.byteArray[3] = QuadroResponse[5];
			}
			delete QuadroResponse;
			delete QuadroRequest;
			QuadroRequest = 0;
			QuadroRequest = 0;
			return 0;
		}
	}
	else
	{
		return -1;
	}
}
int Gsap :: Recover03(byte *_QuadroResposta, Device *_deviceList,uint16_t numsDispositivos) // usado no tratamento do quadro recebido como resposta da requisição do serviço de cód.03
{
	Union2bytesToUint_16 aux0;
    UnionByteToUint_8 aux1;
    int contdDevice = 0;
    int indAddr = 7;
    int indType;
    int indAddr2;
    int indPSS;
    int indJS;
    int indSize;
    int aux2;
    do {
		indType = indAddr + 16;
		indAddr2 = indAddr + 18;
		indPSS = indAddr + 26;
		indJS = indAddr + 27;
		indSize = indAddr + 28;
        // resgata o endereço do dispositivo no quadro
        for(int i = indAddr; i < indAddr + 16; i++)
        {
            _deviceList[contdDevice].Address128[i - indAddr] = _QuadroResposta[i];
        }
        // resgato o tipo do dispositivo
		aux0.byteArray[1] = _QuadroResposta[indType];
        aux0.byteArray[0] = _QuadroResposta[indType + 1];
        _deviceList[contdDevice].deviceType = aux0.uint16;
		// resgato o endereço único
        for(int i = indAddr2; i < indAddr2 + 8; i ++)
        {
            _deviceList[contdDevice].Address64[i - indAddr2] = _QuadroResposta[i];
        }
        // resgato as informações da alimentação do dispositivo
        aux1._byte = _QuadroResposta[indPSS];
        _deviceList[contdDevice].powerSupplyStatus = aux1.uint8;
		// resgato as informações do join do dispositivo
        aux1._byte = _QuadroResposta[indJS];
        _deviceList[contdDevice].joinStatus = aux1.uint8;
        // resgato as "VisibleString" do quadro
        // manufacture
        aux1._byte = _QuadroResposta[indSize];
        _deviceList[contdDevice].manufacture._size = aux1.uint8;
        aux2 = indSize + aux1.uint8 + 1;
		// model
        aux1._byte = _QuadroResposta[aux2];
        _deviceList[contdDevice].model._size = aux1.uint8;
        aux2 = aux2 +aux1.uint8 + 1;
		// revision
        aux1._byte = _QuadroResposta[aux2];
        _deviceList[contdDevice].revision._size = aux1.uint8;
        aux2 = aux2 + aux1.uint8 + 1;
		// deviceTag
        aux1._byte = _QuadroResposta[aux2];
        _deviceList[contdDevice].deviceTag._size = aux1.uint8;
        _deviceList[contdDevice].deviceTag.text = new byte[_deviceList[contdDevice].deviceTag._size];
		if(_deviceList[contdDevice].deviceTag.text == 0)
		{
			return -22;
		}
		else
		{
			for(int i = 0; i < _deviceList[contdDevice].deviceTag._size; i++)
			{
				// passa para a estrutua os bytes refentes ao deviceTag
				_deviceList[contdDevice].deviceTag.text[i] = _QuadroResposta[aux2 + 1 + i];
			}
		}
        aux2 = aux2 + aux1.uint8 + 1;
		// serialNo
        aux1._byte = _QuadroResposta[aux2];
        _deviceList[contdDevice].serialNo._size = aux1.uint8;
        aux2 = aux2 + aux1.uint8 + 1;
        // atualização as variáveis de referêcia
        contdDevice++;
        indAddr = aux2;
    } while (contdDevice<numsDispositivos);
	return 0;
}
int Gsap :: Recover07(byte *_QuadroResposta, NeighborHealth *_neighborHealthList,uint16_t numsVizinhos) // usado no tratamento do quadro recebido como resposta da requisição do serviço de cód.07
{
	Union2ByteToInt_16 aux0;
    UnionByteToUint_8 aux1;
	Union4bytesToUint32 aux2;
    int contdVizinhos = 0;
    int indAddr = 7;
    int indLinkSts;
    int indDPDUsTrans;
    int indDPDUsRece;
    int indDPDUsTransX;
    int indDPDUsReceX;
	int indSignalStren;
	int indSignalQuali;
	do
	{
		indLinkSts = indAddr + 16;
		indDPDUsTrans = indAddr + 17;
		indDPDUsRece = indAddr + 21;
		indDPDUsTransX = indAddr + 25;
		indDPDUsReceX = indAddr + 29;
		indSignalStren = indAddr + 33;
		indSignalQuali = indAddr + 35;
        // resgata o endereço do dispositivo no quadro
        for(int i = indAddr; i < indAddr + 16; i++)
        {
            _neighborHealthList[contdVizinhos].Address128[i - indAddr] = _QuadroResposta[i];
        }
		// resgato o status do link
		aux1._byte = _QuadroResposta[indLinkSts];
		_neighborHealthList[contdVizinhos].linkstatus = aux1.uint8;
		//resgato o valor de DPDUsTransmited
		for(int i = 0; i < 4; i++)
		{
			aux2.byteArray[3-i] = _QuadroResposta[indDPDUsTrans + i];
		}
		_neighborHealthList[contdVizinhos].DPDUsTransmitted = aux2.myInt;
		//resgato o valor de DPDUsReceived
		for(int i = 0; i < 4; i++)
		{
			aux2.byteArray[3-i] = _QuadroResposta[indDPDUsRece + i];
		}
		_neighborHealthList[contdVizinhos].DPDUsReceived = aux2.myInt;
		//resgato o valor de DPDUsTransmitted_X
		for(int i = 0; i < 4; i++)
		{
			aux2.byteArray[3-i] = _QuadroResposta[indDPDUsTransX + i];
		}
		_neighborHealthList[contdVizinhos].DPDUsFailedTransmission = aux2.myInt;
		//resgato o valor de DPDUsReceived_X
		for(int i = 0; i < 4; i++)
		{
			aux2.byteArray[3-i] = _QuadroResposta[indDPDUsReceX + i];
		}
		_neighborHealthList[contdVizinhos].DPDUsFailedReception = aux2.myInt;
		//resgato o valor de signalStrength
		aux0.byteArray[1] = _QuadroResposta[indSignalStren];
		aux0.byteArray[0] = _QuadroResposta[indSignalStren + 1];
		_neighborHealthList[contdVizinhos].signalStrength = aux0.int16;
		//resgato o valor de signalQuality
		aux1._byte = _QuadroResposta[indSignalQuali];
		_neighborHealthList[contdVizinhos].signalQuality = aux1.uint8;
		// atualização variaveis de referêcia
        contdVizinhos++;
        indAddr = indAddr + 36;
	}while(contdVizinhos<numsVizinhos);
	return 0;
}
int Gsap :: Gsap03() // método usado para a requisição do serviço de cód.03 - O retorno do tipo int é o número de dispositivos na rede
{
	if(statusSessao==0)
	{
		if(statusMemory == clean || statusMemory == 0)
		{
			byte aux[34];
			QuadroResponse = new byte[14];
			QuadroRequest = new byte[56];
			if(QuadroResponse == 0 || QuadroRequest == 0)
			{
				return -11;
			}
			else
			{
				// "versao" já esta setado
				serviceCod = 0x03;
				// sessionID já está setado
				transactionID.myInt = 0x02;
				datasize.myInt = 0x26;
				// nessa ponto as partes do meu quadro já estão prontas para serem montadas
				QuadroRequest[0] = versao;
				QuadroRequest[1] = serviceCod;
				// passa o bytes referente ao id da sessão para o quadro
				for(int i = 0; i < 4;i++)
				{
					QuadroRequest[i+2] = sessionID.byteArray[i];
				}
				// passa o bytes referente ao id da transação para o quadro
				for(int i = 0; i < 4;i++)
				{
					QuadroRequest[i+6] = transactionID.byteArray[3-i];
				}
				// passa o bytes referente ao datasize para o quadro
				for(int i = 0; i < 4;i++){
					QuadroRequest[i+10] = datasize.byteArray[3-i];
				}
				// passa o bytes referente ao headerCRC para o quadro
				unionCRC.mylong = crc32c(QuadroRequest,14);
				for(int i = 0; i < 4;i++)
				{
					QuadroRequest[i+14] = unionCRC.myByteArray[i];
				}
				for(int i = 0; i < 34;i++){
					QuadroRequest[i+18] = 0;
					aux[i] = QuadroRequest[i+18]; // faz o uso do vetor auxiliar para calcular o crc
				}
				// passa os bytes referentes ao crc da seq. de zeros
				unionCRC.mylong = crc32c(aux,34);
				for(int i = 0; i < 4;i++){
					QuadroRequest[i+52] = unionCRC.myByteArray[i];
				}
				// nesse ponto o quadro a ser enviado está pronto
				canal->write(QuadroRequest,56);
				canal->readBytes(QuadroResponse,14);	
				if(QuadroResponse[1] == 0x83)
				{
					Union4bytesToUint32 aux;
					for(int i = 0; i < 4; i++)
					{
						aux.byteArray[3-i] = QuadroResponse[i+10];
					}
					unsigned int tama = aux.myInt+4;
					delete QuadroResponse;
					QuadroResponse = 0;
					QuadroResponse = new byte[tama];
					if(QuadroResponse == 0)
					{
							return -11;
					}
					else
					{
						canal->readBytes(QuadroResponse,tama);
						// tratamento da resposta
						Union2bytesToUint_16 aux1;
						aux1.byteArray[1] = QuadroResponse[5];
						aux1.byteArray[0] = QuadroResponse[6];
						DeviceList = new Device[aux1.uint16];
						if(DeviceList == 0)
						{
							return -11;
						}
						else
						{
							clean = 0;
							int statusRecover = Recover03(QuadroResponse,DeviceList,aux1.uint16);
							if(statusRecover==0)
							{
								delete QuadroRequest;
								delete QuadroResponse;
								QuadroRequest = 0;
								QuadroResponse = 0;
								statusMemory = 1;
								setNumDevices(aux1.uint16);
								return 0;
							}
							else
							{
								return statusRecover;
							}
						}
					}
				}
				else
				{
					return -22;
				}
			}
		}
		else
		{
			return -15;
		}
	}
	else
	{
		return -10;
	}
}
void Gsap::clean03(uint16_t numsDispositivos) // Método que deve ser usado logo após a requisição "Gsap03".
{
    /*
	 É feito a liberação da memória alocada no método
	*/
	int check = 0;
	for(int i = 0; i < numsDispositivos; i++)
	{
		delete DeviceList[i].deviceTag.text;
		DeviceList[i].deviceTag.text = 0;
		if(DeviceList[i].deviceTag.text == 0)
		{
			check++;
		}
	}
	delete DeviceList;
	DeviceList = 0;
	if(DeviceList == 0 && check == numsDispositivos)
	{
		clean = 1;
	}
	else
	{
		clean = 0;
	}
}
int Gsap :: Gsap07(byte *NetworkAddress) // método usado para a requisição do serviço de cód.07 - O retorno do tipo int é o número de dispositivos vizinhos ao dispositivo usado na requisição
{
	if(statusSessao==0)
	{
		if(statusMemory == clean || statusMemory == 0)
		{
			// alocação da memória necessária para os quadros
			QuadroRequest = new byte[56];
			QuadroResponse = new byte[14];
			// checagem da alocação da memória
			if(QuadroRequest == 0 || QuadroResponse == 0)
			{
				return -22;
			}
			else
			{
				// montagem do quadro para ser enviada
				// versao ja esta setada
				serviceCod = 0x07;
				// sessionID ja esta setado
				//seta transactionID
				transactionID.myInt = 0x03;
				//seta datasize
				datasize.myInt = 0x26;
				// nessa ponto as partes do meu quadro já estão prontas para serem montadas
				QuadroRequest[0] = versao;
				QuadroRequest[1] = serviceCod;
				// passa o bytes referente ao id da sessão para o quadro
				for(int i = 0; i < 4;i++)
				{
					QuadroRequest[i+2] = sessionID.byteArray[i];
				}
				// passa o bytes referente ao id da transação para o quadro
				for(int i = 0; i < 4;i++)
				{
					QuadroRequest[i+6] = transactionID.byteArray[3-i];
				}
				// passa o bytes referente ao datasize para o quadro
				for(int i = 0; i < 4;i++){
					QuadroRequest[i+10] = datasize.byteArray[3-i];
				}
				// passa o bytes referente ao headerCRC para o quadro
				unionCRC.mylong = crc32c(QuadroRequest,14);
				for(int i = 0; i < 4;i++)
				{
					QuadroRequest[i+14] = unionCRC.myByteArray[i];
				}
				// passa os bytes referentes ao endereço do dispositivo
				for(int i = 0; i < 34; i++)
				{
					QuadroRequest[i+18] = NetworkAddress[i];
				}
				// passa o bytes referente ao dataCRC para o quadro
				unionCRC.mylong = crc32c(NetworkAddress,34);
				for(int i = 0; i < 4;i++)
				{
					QuadroRequest[i+52] = unionCRC.myByteArray[i];
				}
				// quadro montado, pronto para envio
				canal->write(QuadroRequest,56);
				canal->readBytes(QuadroResponse,14);	
				if(QuadroResponse[1] == 0x87)
				{
					Union4bytesToUint32 aux;
					for(int i = 0; i < 4; i++)
					{
						aux.byteArray[3-i] = QuadroResponse[i+10];
					}
					unsigned int tama = aux.myInt+4;
					delete QuadroResponse;
					QuadroResponse = 0;
					// faço a segunda alocação
					QuadroResponse = new byte[tama];
					if(QuadroResponse == 0)
					{
							return -22;
					}
					else
					{
						canal->readBytes(QuadroResponse,tama);
						// tratamento da resposta
						Union2bytesToUint_16 aux1;
						aux1.byteArray[1] = QuadroResponse[5];
						aux1.byteArray[0] = QuadroResponse[6];
						NeighborHealthList = new NeighborHealth[aux1.uint16];
						// atribuo clean = 0 para indicar que será feito uma alocação que necessita ter a memoria limpa pelo usuario
						clean = 0;
						Recover07(QuadroResponse,NeighborHealthList,aux1.uint16);
						setNumLinks(aux1.uint16);
						delete QuadroRequest;
						delete QuadroResponse;
						QuadroRequest = 0;
						QuadroResponse = 0;
						statusMemory = 1;
						return 0;
					}
				}
				else
				{
					return -22;
				}
			}
		}
		else
		{
			return -15;
		}
	}
	else
	{
		return -10;
	}
}
void Gsap :: clean07() // Método que deve ser usado logo após a requisição "Gsap07".
{
	delete NeighborHealthList;
	NeighborHealthList = 0;
	if(!NeighborHealthList)
	{
		clean = 1;
	}
	else
	{
		clean = 0;
	}
}

