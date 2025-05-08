#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "neopixel.c"

// Pino e canal do microfone no ADC.
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)

// Parâmetros e macros do ADC.
#define ADC_CLOCK_DIV 96.f
#define SAMPLES 200 // Número de amostras que serão feitas do ADC.
#define ADC_ADJUST(x) (x * 3.3f / (1 << 12u) - 1.65f) // Ajuste do valor do ADC para Volts.
#define ADC_MAX 3.3f
#define ADC_STEP (3.3f/5.f) // Intervalos de volume do microfone.

// Pino e número de LEDs da matriz de LEDs.
#define LED_PIN 7
#define LED_COUNT 25

#define abs(x) ((x < 0) ? (-x) : (x))

// Canal e configurações do DMA
uint dma_channel;
dma_channel_config dma_cfg;

// Buffer de amostras do ADC.
uint16_t adc_buffer[SAMPLES];

void sample_mic();

int main() {
  stdio_init_all();

  // Delay para o usuário abrir o monitor serial...
  sleep_ms(5000);

  // Preparação da matriz de LEDs.
  printf("Preparando NeoPixel...");
  
  npInit(LED_PIN, LED_COUNT);

  // Preparação do ADC.
  printf("Preparando ADC...\n");

  adc_gpio_init(MIC_PIN);
  adc_init();
  adc_select_input(MIC_CHANNEL);

  adc_fifo_setup(
    true, // Habilitar FIFO
    true, // Habilitar request de dados do DMA
    1, // Threshold para ativar request DMA é 1 leitura do ADC
    false, // Não usar bit de erro
    false // Não fazer downscale das amostras para 8-bits, manter 12-bits.
  );

  adc_set_clkdiv(ADC_CLOCK_DIV);

  printf("ADC Configurado!\n\n");

  printf("Preparando DMA...");

  // Tomando posse de canal do DMA.
  dma_channel = dma_claim_unused_channel(true);

  // Configurações do DMA.
  dma_cfg = dma_channel_get_default_config(dma_channel);

  channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16); // Tamanho da transferência é 16-bits (usamos uint16_t para armazenar valores do ADC)
  channel_config_set_read_increment(&dma_cfg, false); // Desabilita incremento do ponteiro de leitura (lemos de um único registrador)
  channel_config_set_write_increment(&dma_cfg, true); // Habilita incremento do ponteiro de escrita (escrevemos em um array/buffer)
  
  channel_config_set_dreq(&dma_cfg, DREQ_ADC); // Usamos a requisição de dados do ADC

  // Amostragem de teste.
  printf("Amostragem de teste...\n");
  sample_mic();

  printf("Configuracoes completas!\n");

  printf("\n----\nIniciando loop...\n----\n");
  while (true) {

    // Realiza uma amostragem do microfone.
    sample_mic();

    // Usa a última amostra do ADC para determinar a intensidade
    uint16_t adc_value = adc_buffer[SAMPLES - 1]; // Pega o último valor lido do buffer
    float voltage = ADC_ADJUST(adc_value); // Ajuste para a faixa de 0-3.3V
    printf("ADC Value: %d, Voltage: %.4f\n", adc_value, voltage);  // Imprime o valor no Serial Monitor

    // Mapeia o valor do ADC diretamente para a intensidade dos LEDs
    uint intensity = (uint)(voltage * 5);  // A intensidade será entre 0 e 4, mapeada de acordo com o valor do ADC

    // Limpa a matriz de LEDs.
    npClear();

    // A depender da intensidade do som, acende LEDs específicos.
    switch (intensity) {
      case 0: 
        // Som muito baixo: LEDs apagados
        break; 
      case 1:
        // Muito baixo: Verde
        npSetLED(12, 0, 255, 0); // Acende o centro de verde
        break;
      case 2:
        // Baixo: Amarelo
        npSetLED(12, 255, 255, 0); // Acende o centro de amarelo
        // Primeiro anel: Amarelo
        npSetLED(7, 255, 255, 0);
        npSetLED(11, 255, 255, 0);
        npSetLED(13, 255, 255, 0);
        npSetLED(17, 255, 255, 0);
        break;
      case 3:
        // Médio: Ciano
        npSetLED(12, 0, 255, 255); // Acende o centro de ciano
        // Primeiro anel: Ciano
        npSetLED(7, 0, 255, 255);
        npSetLED(11, 0, 255, 255);
        npSetLED(13, 0, 255, 255);
        npSetLED(17, 0, 255, 255);
        // Segundo anel: Ciano
        npSetLED(2, 0, 255, 255);
        npSetLED(6, 0, 255, 255);
        npSetLED(8, 0, 255, 255);
        npSetLED(10, 0, 255, 255);
        npSetLED(14, 0, 255, 255);
        npSetLED(16, 0, 255, 255);
        npSetLED(18, 0, 255, 255);
        npSetLED(22, 0, 255, 255);
        break;
      case 4:
        // Acima do médio: Azul
        npSetLED(12, 0, 0, 255); // Acende o centro de azul
        // Primeiro anel: Azul
        npSetLED(7, 0, 0, 255);
        npSetLED(11, 0, 0, 255);
        npSetLED(13, 0, 0, 255);
        npSetLED(17, 0, 0, 255);
        // Segundo anel: Azul
        npSetLED(2, 0, 0, 255);
        npSetLED(6, 0, 0, 255);
        npSetLED(8, 0, 0, 255);
        npSetLED(10, 0, 0, 255);
        npSetLED(14, 0, 0, 255);
        npSetLED(16, 0, 0, 255);
        npSetLED(18, 0, 0, 255);
        npSetLED(22, 0, 0, 255);
        break;
      case 5:
        // Som alto: Vermelho
        npSetLED(12, 255, 0, 0); // Acende o centro de vermelho
        // Primeiro anel: Vermelho
        npSetLED(7, 255, 0, 0);
        npSetLED(11, 255, 0, 0);
        npSetLED(13, 255, 0, 0);
        npSetLED(17, 255, 0, 0);
        // Segundo anel: Vermelho
        npSetLED(2, 255, 0, 0);
        npSetLED(6, 255, 0, 0);
        npSetLED(8, 255, 0, 0);
        npSetLED(10, 255, 0, 0);
        npSetLED(14, 255, 0, 0);
        npSetLED(16, 255, 0, 0);
        npSetLED(18, 255, 0, 0);
        npSetLED(22, 255, 0, 0);
        break;
    }
    // Atualiza a matriz.
    npWrite();

    // Envia a intensidade e a média das leituras do ADC por serial.
    printf("Intensity: %d, Voltage: %.4f\r", intensity, voltage);
  }
}

/**
 * Realiza as leituras do ADC e armazena os valores no buffer.
 */
void sample_mic() {
  adc_fifo_drain(); // Limpa o FIFO do ADC.
  adc_run(false); // Desliga o ADC (se estiver ligado) para configurar o DMA.

  dma_channel_configure(dma_channel, &dma_cfg,
    adc_buffer, // Escreve no buffer.
    &(adc_hw->fifo), // Lê do ADC.
    SAMPLES, // Faz SAMPLES amostras.
    true // Liga o DMA.
  );

  // Liga o ADC e espera acabar a leitura.
  adc_run(true);
  dma_channel_wait_for_finish_blocking(dma_channel);
  
  // Acabou a leitura, desliga o ADC de novo.
  adc_run(false);
}
