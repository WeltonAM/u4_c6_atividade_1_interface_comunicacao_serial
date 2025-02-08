# Atividade 2 - Unidade 4 - Capítulo 6 - Interfaces de Comunicação Serial com RP2040

Este projeto foi desenvolvido como parte da **atividade prática individual** do estudo sobre **interfaces de comunicação serial no RP2040**, **Atividade 1** do curso de **EmbarcaTech TIC37 - Unidade 4**, **Capítulo 6**, e explora as funcionalidades da placa **BitDogLab**. O objetivo principal é integrar hardware e software para manipulação de LEDs e interação via UART e I2C.

## 📌 Objetivos

- Compreender o funcionamento e a aplicação de comunicação serial em microcontroladores.
- Aplicar os conhecimentos adquiridos sobre **UART e I2C** na prática.
- Manipular e controlar **LEDs comuns e LEDs WS2812**.
- Fixar conceitos sobre **botões de acionamento, interrupções e Debounce**.
- Desenvolver um **projeto funcional** combinando hardware e software.

---

## 🔧 Descrição do Projeto

O projeto envolve o uso dos seguintes componentes conectados à **placa BitDogLab**:

- **Matriz 5x5 de LEDs WS2812** conectada à **GPIO 7**.
- **LED RGB** com pinos conectados às **GPIOs 11, 12 e 13**.
- **Botão A** conectado à **GPIO 5**.
- **Botão B** conectado à **GPIO 6**.
- **Display SSD1306** conectado via **I2C (GPIO 14 e GPIO 15)**.

---

## ⚙️ Funcionalidades do Projeto

### 1️⃣ Modificação da Biblioteca `font.h`

- Adicionar **caracteres minúsculos** à biblioteca `font.h`.

### 2️⃣ Entrada de caracteres via PC

- Utilizar o **Serial Monitor** do VS Code para **digitação de caracteres**.
- O caractere digitado deve ser **exibido no display SSD1306**.
- Caso um **número de 0 a 9** seja digitado, um **símbolo correspondente** deve ser exibido na matriz **5x5 WS2812**.

### 3️⃣ Interação com o **Botão A**

- Pressionar o botão A **alterna o estado do LED Verde** (ligado/desligado).
- O estado do LED deve ser **registrado de duas formas**:
  - Uma **mensagem informativa** no **display SSD1306**.
  - Um **texto descritivo** no **Serial Monitor**.

### 4️⃣ Interação com o **Botão B**

- Pressionar o botão B **alterna o estado do LED Azul** (ligado/desligado).
- O estado do LED deve ser **registrado de duas formas**:
  - Uma **mensagem informativa** no **display SSD1306**.
  - Um **texto descritivo** no **Serial Monitor**.

---

## 📋 Requisitos do Projeto

Para o desenvolvimento, devem ser seguidos os seguintes requisitos:

1. **Uso de interrupções**: Todas as funcionalidades dos botões devem ser implementadas com **rotinas de interrupção (IRQ)**.
2. **Debouncing**: Implementar tratamento de **bouncing** via software.
3. **Controle de LEDs**: Utilizar **LEDs comuns e WS2812**, demonstrando domínio sobre diferentes tipos de controle.
4. **Uso do Display SSD1306**:
   - Implementar **fontes maiúsculas e minúsculas**.
   - Demonstrar **compreensão do protocolo I2C**.
5. **Envio de informação via UART**:
   - Validar a comunicação serial via **UART**.

---

## 🚀 Como Executar o Projeto

### 📥 Clone o repositório:

```bash
git clone https://github.com/WeltonAM/u4_c6_atividade_1_interface_comunicacao_serial
```

### 📂 Acesse a pasta do projeto:

```bash
cd u4_c6_atividade_1_interface_comunicacao_serial
```

### 🛠️ Instale as dependências:

- Raspberry Pi Pico SDK
- Extensões do VS Code para RP2040
- Biblioteca **ssd1306** para manipulação do display OLED
- Biblioteca **ws2812** para controle da matriz de LEDs endereçáveis

### ▶️ Compile e execute:

- Utilize a ferramenta de **upload** no **VS Code** para carregar o código na placa BitDogLab.
- Acompanhe a execução através do **Serial Monitor**.

### Para experimentar no simulador do Wokwi, altere os seguintes códigos:

```c
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (num_map[num][i] == 1)
        {
            // led_buffer[i] = random_color(); // Cores aleatórias para testes na placa BitDogLab
            led_buffer[i] = urgb_u32(255, 0, 0); // Leds mais fortes para teste no simulador Wokwi
        }
    }
```

---

## 🎥 Demonstração

[![Assistir Vídeo](https://img.shields.io/badge/Assistir%20Vídeo-Demonstrativo-blue?style=for-the-badge&logo=youtube)](https://drive.google.com/file/d/1y22qQr478hn4p2j-CESjJ-Ike40zO8zz/view?usp=sharing)

## 📌 Conclusão

Este projeto consolidou os conceitos de **UART, I2C, interrupções, debounce e controle de LEDs** em um microcontrolador RP2040. A implementação prática desses conceitos demonstrou a importância da integração entre **hardware e software** para o desenvolvimento de sistemas embarcados eficientes.

💡 **Agora é sua vez! Explore, modifique e amplie o projeto para aprender ainda mais!**