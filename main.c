#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"//biblioteca para funções  pwm
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include <stdio.h>
//matriz led
#include "hardware/pio.h"
#include "ws2812.pio.h"
#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7
// Variável global para armazenar a cor (Entre 0 e 255 para intensidade)
uint8_t led_r = 5;  // Intensidade do vermelho
uint8_t led_g = 5; // Intensidade do verde
uint8_t led_b = 5; // Intensidade do azul 
bool led_buffer[NUM_PIXELS];// Variável (protótipo)

//protótipo funções que ligam leds da matriz 5x5
static inline void put_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void set_one_led(uint8_t r, uint8_t g, uint8_t b);//liga os LEDs escolhidos 


#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define led1 11//definindo LED verde
#define led3 13//definindo LED vermelho 
#define botaoA 5//definindo botão A
#define buzzer 21// pino do Buzzer na BitDogLab

bool modo=1;//Flag para modo (1 diurno e 0 noturno)

static volatile uint32_t last_time_A = 0; // Armazena o tempo do último evento para Bot B(em microssegundos)
uint Cor_sinal=4;//auxiliar para com do sinal (Verde=1, Amarelo=2, Vermelho=3)
void vLedRGBTask()//task para LED RGB e Matriz
{
    //definindo LED verde
    gpio_init(led1);
    gpio_set_dir(led1, GPIO_OUT);
    
    //definindo LED vermelho
    gpio_init(led3);
    gpio_set_dir(led3, GPIO_OUT);
    while (true)
    {   
        if(modo){//modo diurno
            for(int i=0;i<4;i++){//para verde (total 4 segundos)
                Cor_sinal=1;
                led_r=0;//atualiza cores Matriz
                led_g=5;
                set_one_led(led_r, led_g, 0);//imprime cor matriz
                gpio_put(led1, true);
                vTaskDelay(pdMS_TO_TICKS(500));//beep curto por segundo 
                gpio_put(led1, false);
                vTaskDelay(pdMS_TO_TICKS(500));
                if(modo==0){
                    break;
                }
            }
            for(int i=0;i<10;i++){//para amarelo (Vermelho e verde ligados simultaneamente)(total 4 segundos)
                Cor_sinal=2;
                led_r=5;
                led_g=5;
                set_one_led(led_r, led_g, 0);
                if(modo==0){
                    break;
                }
                gpio_put(led1, true);
                gpio_put(led3, true);
                vTaskDelay(pdMS_TO_TICKS(200));//beep rápido intermitente 
                gpio_put(led1, false);
                gpio_put(led3, false);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            for(int i=0;i<2;i++){//para vermelho (total 4 segundos)
                Cor_sinal=3;
                led_r=5;
                led_g=0;
                set_one_led(led_r, led_g, 0);
                if(modo==0){
                    break;
                }
                gpio_put(led3, true);
                vTaskDelay(pdMS_TO_TICKS(500));//tom continuo curto 
                gpio_put(led3, false);
                vTaskDelay(pdMS_TO_TICKS(1500));
            }
        }else{//modo noturno (amarelo lento a cada 2s )
            Cor_sinal=2;
            led_r=5;
            led_g=5;
            set_one_led(led_r, led_g, 0);//atualiza cor matriz 
            gpio_put(led1, true);
            gpio_put(led3, true);
            vTaskDelay(pdMS_TO_TICKS(1500));
            gpio_put(led1, false);
            gpio_put(led3, false);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void vBuzzerTask()
{
     //configurando PWM
    uint pwm_wrap = 8000;// definindo valor de wrap referente a 12 bits do ADC
    gpio_set_function(buzzer, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(buzzer);
    pwm_set_wrap(slice_num, pwm_wrap);
    pwm_set_clkdiv(slice_num, 125.0);//divisor de clock 
    pwm_set_enabled(slice_num, true);
    //gpio_init(led2);
    //gpio_set_dir(led2, GPIO_OUT);
    while (true)
    {
        if(modo==1){
            if(Cor_sinal==1){//sinal verde
                pwm_set_gpio_level(buzzer, 400);//10% de Duty cycle
                vTaskDelay(pdMS_TO_TICKS(500));
                pwm_set_gpio_level(buzzer, 0);
                vTaskDelay(pdMS_TO_TICKS(500));
            }else if(Cor_sinal==2){//sinal amarelo 
                pwm_set_gpio_level(buzzer, 400);//10% de Duty cycle
                vTaskDelay(pdMS_TO_TICKS(200));
                pwm_set_gpio_level(buzzer, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }else if(Cor_sinal==3){//sinal vermelho
                pwm_set_gpio_level(buzzer, 400);//10% de Duty cycle
                vTaskDelay(pdMS_TO_TICKS(500));
                pwm_set_gpio_level(buzzer, 0);
                vTaskDelay(pdMS_TO_TICKS(1500));
            }
        }else{//amarelo noturno
            pwm_set_gpio_level(buzzer, 400);//10% de Duty cycle
            vTaskDelay(pdMS_TO_TICKS(1500));
            pwm_set_gpio_level(buzzer, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

void vDisplay3Task()//Task Para imprimir no display e atualizar Flag modo
{
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display
    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    char str_y[8]={'D','I','U','R','N','O','\0'}; // Buffer para armazenar a string
    char str_x[8]={'N','O','T','U','R','N','O','\0'};
    char str_R[6]="pare\0";//mensagem sinal vermelho
    char str_Y[9]="atencao\0";//mensagem sinal amarelo
    char str_G[17]="pode atravessar\0";//mensagem sinal verde
    bool cor = true; 
    while (true)
    {
        uint32_t current_time = to_us_since_boot(get_absolute_time());//// Obtém o tempo atual em microssegundos
        if(gpio_get(botaoA)==0 &&(current_time - last_time_A) > 200000){//200ms de boucing adiconado como condição
            last_time_A = current_time; // Atualiza o tempo do último evento
            modo=!modo;//mudo estado Flag
        }
        ssd1306_fill(&ssd, !cor);                          // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo
        ssd1306_line(&ssd, 3, 25, 123, 25, cor);           // Desenha uma linha
        ssd1306_line(&ssd, 3, 37, 123, 37, cor);           // Desenha uma linha
        ssd1306_draw_string(&ssd, "SEMAFORO", 29, 6); // Desenha uma string
        ssd1306_draw_string(&ssd, "INTELIGENTE", 20, 16);  // Desenha uma string
        if(Cor_sinal==1){//mensagem se verde
            ssd1306_draw_string(&ssd, str_G, 4, 28); // Desenha uma string
        }else if(Cor_sinal==2){//mensagem se amarelo
            ssd1306_draw_string(&ssd, str_Y, 35, 28); // Desenha uma string
        }else if(Cor_sinal==3){//mensagem se vermelhho
            ssd1306_draw_string(&ssd, str_R, 41, 28); // Desenha uma string
        }
        ssd1306_draw_string(&ssd, "MODO", 45, 41);    // Desenha uma string
        if(modo){
            ssd1306_draw_string(&ssd, str_y, 38, 52);          // Desenha uma string
        }else{
            ssd1306_draw_string(&ssd, str_x, 33, 52);
        }
        ssd1306_send_data(&ssd);                           // Atualiza o display
    }
}

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void vGpio_irq_handler(uint gpio, uint32_t events)//task para botões 
{
    if(gpio_get(botaoB)==0){
        reset_usb_boot(0, 0);
    }
}

int main()
{
    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(botaoB);
    gpio_set_dir(botaoB, GPIO_IN);
    gpio_pull_up(botaoB);
    //Para alterar a Flag Botão A
    gpio_init(botaoA);
    gpio_set_dir(botaoA, GPIO_IN);
    gpio_pull_up(botaoA);
    
    gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &vGpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botaoA, GPIO_IRQ_EDGE_FALL, true, &vGpio_irq_handler);
    // Fim do trecho para modo BOOTSEL com botão B

    //configuração PIO
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    stdio_init_all();

    xTaskCreate(vLedRGBTask, "Task Led1", configMINIMAL_STACK_SIZE,
         NULL, tskIDLE_PRIORITY, NULL);//task Cores LEDs
    xTaskCreate(vBuzzerTask, "Task Buzzer2", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);//Task para alerta sonoro Buzzer
    xTaskCreate(vDisplay3Task, "Task Disp3", configMINIMAL_STACK_SIZE, 
        NULL, tskIDLE_PRIORITY, NULL);//Task para imprimir no Display 
    vTaskStartScheduler();
    panic_unsupported();
}
bool led_buffer[NUM_PIXELS] = { //Buffer para armazenar quais LEDs estão ligados matriz 5x5
    0, 0, 0, 0, 0,
    0, 1, 1, 1, 0,
    0, 1, 1, 1, 0,
    0, 1, 1, 1, 0,
    0, 0, 0, 0, 0};
static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}
void set_one_led(uint8_t r, uint8_t g, uint8_t b)
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0); // Desliga os LEDs com zero no buffer
        }
    }
}