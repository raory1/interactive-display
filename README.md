
# Display OLED com Entrada UART
Projeto realizado como parte do estudo sobre o protocolo I²C utilizando o kit BitDogLab.
Permite o controle de uma matriz de LEDs 5x5 e de um display OLED SSD1306 utilizando comandos enviados via comunicação serial UART.  Exibe diferentes padrões na matriz de LEDs e também atualiza o display com texto dinâmico baseado na entrada recebida.

## Features
- Controle de uma matriz de LEDs para exibição de números de 0 a 9
- Display OLED: Exibe texto dinâmico incluindo a entrada recebida via UART.
-  Implementação de interrupções e debounce para resposta rápida e precisa dos botões

## Requisitos de Hardware e configuração dos Pinos
- Raspberry Pi Pico W.
- UART TX: Pino 0
- UART RX: Pino 1
- Botão A, conectado ao pino GPIO 5.
- Botão B, conectado ao pino GPIO 6.
- Matriz de LEDs (5x5 WS2812), conectada ao pino GPIO 7.
- LEDs (verde, conectado ao pino GPIO 11; azul, conectado ao pino GPIO 13; vermelho, conectado ao pino GPIO 13).
- I2C SDA: Pino 14
- I2C SCL: Pino 15

## Dependências
- SDK Raspberry Pi Pico

## Utilização
- Padrões da Matriz de LEDs: Digite qualquer dígito [0-9] no terminal serial, e o padrão correspondente será exibido na matriz de LEDs 5x5.
- Display OLED: O OLED exibirá o caractere que você digitou, além de outros textos dinâmicos.
- Botões: Pressione o botão A ou B para alternar o LED correspondente (Verde ou Azul), e o OLED exibirá uma mensagem sobre o status do LED.

## Demonstração do funcionamento
https://www.youtube.com/watch?v=U7Y2uO8jC-E