# Biblioteca_GSAP_ESP_comunication
Biblioteca para comunicação do ESP com o gateway da yokogawa via GSAP(suporte a requisições de código gsap: 01,03,07), funcional porém com alguns detalhes.
A biblioteca está funcional porém o único teste feito foi na execução de um código para controle de uma planta com dois tanques no LAMP(Laboratório da UFRN), usando o ESP32 como controlador.
O ESP fazia a requisição dos parâmetros de controle via MODBUS ao Gateway da YOKOGAWA, parâmetros da rede via GSAP, usando está biblioteca, e execução do controle. 
A execução apenas da requisição dos parâmetros de rede resultava em um tempo de duração de execução do código no ESP sem travamentos de aproximadamente 1 hora.
Para usar o método de requisição de código 07 do padrão GSAP é necessário ser passado como parâmetro um ponteiro que aponta para um array por exemplo, com o endereço do dispositivo ao qual se deseja ter os parâmetros dos links que o mesmo está inserido,
endereço esse diferente do retornado pela requisição do código 03 do padrão GSAP.
