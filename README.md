# Simulador de Semáforo

Este projeto foi desenvolvido como parte da etapa de **nivelamento** do Programa **EmbarcaTech**. O objetivo desta fase é consolidar e reforçar os conhecimentos adquiridos durante a capacitação em **Sistemas Embarcados** (SE), ajudando os alunos a aplicarem esses conhecimentos de forma prática em projetos reais. 

## Descrição do Projeto

O **Simulador de Semáforo** é uma aplicação que treina as reações do usuário conforme as cores do semáforo. O projeto utiliza uma placa **BitDogLab** e consiste em uma interface simples e intuitiva, com feedback visual para as ações do usuário e exibição de resultados em um **display OLED**.

### Funcionalidade

- **Semáforo Verde**: O usuário deve pressionar o botão A para acelerar.
- **Semáforo Vermelho**: O usuário deve pressionar o botão B para frear.
- **Semáforo Amarelo**: O usuário não deve pressionar nenhum botão.

A pontuação é atualizada a cada ação correta e o feedback do jogo é mostrado no display OLED. O sistema também conta o tempo de cada semáforo e controla as transições entre os estados.

## Objetivo do Projeto

O objetivo deste projeto foi consolidar os conhecimentos adquiridos durante a capacitação sobre Sistemas Embarcados, usando linguagem C para programar microcontroladores (neste caso, a BitDogLab, baseada no Raspberry Pi Pico W). O projeto me permitiu praticar a programação de sistemas embarcados, com ênfase no uso de botões, LEDs RGB, display OLED e matriz de LEDs.

### Relevância do Projeto

Este simulador é relevante para iniciantes que desejam compreender como a lógica de controle de sistemas embarcados pode ser aplicada em situações cotidianas, como o trânsito. O projeto oferece uma maneira divertida e educativa de se familiarizar com o controle de dispositivos em sistemas embarcados.

## Hardware Utilizado

### Placa BitDogLab

A **BitDogLab** é uma plataforma de desenvolvimento baseada no **Raspberry Pi Pico W**, projetada para aplicações de **Internet das Coisas (IoT)** e sistemas embarcados. Ela oferece diversos recursos integrados, facilitando o desenvolvimento de projetos complexos.

### Componentes Utilizados:

- **Botões A e B**: Capturam a entrada do usuário para as ações de "acelerar" e "frear".
- **LEDs RGB**: Indicadores de estado do semáforo (verde, vermelho e amarelo).
- **Matriz de LEDs**: Representa visualmente o semáforo com 25 LEDs dispostos em uma grade de 5x5.
- **Display OLED**: Exibe informações sobre a pontuação, o estado do semáforo e feedback das ações do usuário.

### Pinagem

- **Botão A**: GPIO 5 (entrada com pull-up interno).
- **Botão B**: GPIO 6 (entrada com pull-up interno).
- **LED RGB**: Pino 11 controla o LED verde e Pino 13 controla o LED vermelho.
- **Matriz de LEDs**: Controlada pelo pino 7 da BitDogLab, utilizando o protocolo WS2812.

### Display OLED

- **Conexões I2C**:
  - **SDA (Data)**: GPIO 14
  - **SCL (Clock)**: GPIO 15
  - **Endereço I2C**: 0x3C

## Funcionamento do Firmware

O firmware foi desenvolvido para controlar os LEDs, ler os botões e atualizar o display OLED com a pontuação e o estado do semáforo. O fluxo de operação segue os seguintes passos:

1. **Início**: O jogo começa aguardando que ambos os botões A e B sejam pressionados.
2. O semáforo é exibido no display OLED e o sistema aguarda a interação do usuário.
3. Dependendo do estado do semáforo:
   - **Verde**: O usuário deve pressionar o botão A para acelerar.
   - **Vermelho**: O usuário deve pressionar o botão B para frear.
   - **Amarelo**: O usuário não deve pressionar nenhum botão.
4. O sistema valida as ações, contabiliza pontos e fornece feedback.
5. Se a pontuação cair para zero ou abaixo, o sistema exibe "Você perdeu" e reinicia o jogo.

### Ações Validáveis

- **Acelerar**: O botão A é válido apenas quando o semáforo estiver verde.
- **Frear**: O botão B é válido apenas quando o semáforo estiver vermelho.
- **Esperar no Amarelo**: O jogador deve aguardar sem pressionar nenhum botão quando o semáforo estiver amarelo.

## Como Rodar o Projeto

1. **Clone o repositório**:
    ```bash
    git clone https://github.com/MatheusGODZILLA/simulador-semaforo.git
    cd simulador-semaforo
    ```

2. **Compile e faça o upload do código para a BitDogLab**:
    - Utilize o **Pico SDK** para compilar e transferir o código para a BitDogLab.

3. **Conecte os componentes** conforme as configurações de pinos descritas anteriormente.

4. **Execute o simulador**:
    - Ao ligar o dispositivo, o jogo começará automaticamente. Pressione os botões A e B conforme os sinais do semáforo para treinar suas reações!

## Melhorias Futuras

Embora o simulador funcione corretamente, existem melhorias que podem ser implementadas:

- **Efeitos Sonoros**: Adicionar sons para cada ação correta ou incorreta.
- **Novos Desafios**: Incluir novos desafios, como tempos de reação mais rápidos ou combinações de ações.
- **Interface Gráfica**: Melhorar a interface gráfica no display OLED para tornar o jogo mais interativo e dinâmico.

## Links

- **Vídeo de funcionamento do projeto**: [Simulador de Semáforo - Nivelamento – Residentes EmbarcaTech](https://www.youtube.com/watch?v=Rn8ugAWXUa8)
- **Repositório BitDogLab com exemplos de códigos**: [BitDogLab-C](https://github.com/BitDogLab/BitDogLab-C)
