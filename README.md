# U1A5
Implementação de um sistema IoT utilizando a placa BitDogLab como cliente MQTT, com comunicação via protocolo MQTT usando o broker Mosquitto em um celular Android (Termux). O projeto integra sensores e controle remoto com visualização via IoT MQTT Panel (Android) e um cliente adicional em PC para monitoramento.

__Aluno:__
Lucas Carneiro de Araújo Lima

## ATIVIDADE 

__Para este trabalho, os seguintes componentes e ferramentas se fazem necessários:__
1) Microcontrolador Raspberry Pi Pico W.
2) Ambiente de trabalho VSCode.
3) LEDs RGB.
4) _Display_ SSD1306.
5) 3 Botões Pull-Up.
7) Ferramenta educacional BitDogLab.
8) FreeRTOS
9) 6 LEDs
10) 6 Resistores de 330 ohms
11) 1 resistor de 5100 ohms
12) 1 resistor de 10000 ohms
13) 1 Sensor DHT11
14) 1 LDR

__O resultado do projeto pode ser assistido através deste link: [Vídeo de Apresentação](https://youtu.be/1CXJDWMVcv0).__

## Instruções de Uso

### 1. Clone o repositório
Abra o terminal e execute o comando abaixo para clonar o repositório em sua máquina:
```bash
git clone https://github.com/LucasCarneiro3301/U1A5.git
```

### 2. Configure o ambiente de desenvolvimento
Certifique-se de que o [SDK do Raspberry Pi Pico](https://github.com/raspberrypi/pico-sdk) esteja instalado e configurado corretamente no seu sistema.

### 3. Instale o CMake
Certifique-se de que o [CMake](https://cmake.org/download/) esteja instalado e configurado corretamente no seu sistema.

### 4. Clone o repositório do FreeRTOS
Certifique-se de clonar o repositório do FreeRTOS. Repositório: [FreeRTOS](https://github.com/FreeRTOS/FreeRTOS-Kernel.git).

### 5. Conexão com a Rapberry Pico
1. Conecte o Raspberry Pi Pico ao seu computador via USB.
2. Inicie o modo de gravação pressionando o botão **BOOTSEL** e **RESTART**.
3. O Pico será montado como um dispositivo de armazenamento USB.
4. Execute através do comando **RUN** a fim de copiar o arquivo `U1A5.uf2` para o Pico.
5. O Pico reiniciará automaticamente e executará o programa.

### 5. Observações (IMPORTANTE !!!)
2. Manuseie a placa com cuidado.

## Recursos e Funcionalidades

### 1. Botão

| BOTÃO                            | DESCRIÇÃO                                     | 
|:----------------------------------:|:---------------------------------------------:|
| A                                  | Permite parar ou reiniciar a estufa localmente              | 

### 2. Display OLED
Monitora a temperatura, umidade, luminosidade e qualidade geral do sistema.

### 3. LED RGB
Representa visualmente a qualidade da estufa.

### 4. BUZZER
Sinaliza através de efeitos sonoros que a parada do sistema foi solicitada.

### 5. Módulo Wi-Fi CYW43
Fornece conectividade Wi-Fi ao Raspberry Pi Pico W.

### 6. Comunicação MQTT
Se conecta a uma dashboard externa através de comunicação MQTT
